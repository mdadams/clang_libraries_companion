#include <format>
#include <memory>
#include <string>
#include <utility>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

#include "AstDumper.hpp"
#include "AstNodeCounterVisitor.hpp"
#include "TreeFormatter.hpp"

namespace ct = clang::tooling;
namespace lc = llvm::cl;

static lc::opt<bool> clVerbose("verbose");
static bool clEnableLastChild = true;
static lc::opt<bool> clDummy1("enable-last-child",
  lc::callback([](const bool&){clEnableLastChild = true;}));
static lc::opt<bool> clDummy2("no-enable-last-child",
  lc::callback([](const bool&){clEnableLastChild = false;}));
static bool clFlushLeft = true;
static lc::opt<bool> clDummy3("flush-left",
  lc::callback([](const bool&){clFlushLeft = true;}));
static lc::opt<bool> clDummy4("no-flush-left",
  lc::callback([](const bool&){clFlushLeft = false;}));

class MyAstConsumer : public clang::ASTConsumer {
public:
	MyAstConsumer(clang::CompilerInstance& compInstance,
	  const std::string& fileName) : compInstance_(&compInstance),
	  fileName_(fileName) {}
	void HandleTranslationUnit(clang::ASTContext& astContext) override {
		clang::SourceManager& sourceManager = compInstance_->getSourceManager();
		const clang::LangOptions& langOpts = compInstance_->getLangOpts();
		clang::TranslationUnitDecl* tuDecl = astContext.getTranslationUnitDecl();
#if 1
		{
			llvm::outs() << "RecursiveASTVisitor starting\n";
			AstNodeCounterVisitor visitor;
			visitor.TraverseDecl(tuDecl);
			llvm::outs() << "RecursiveASTVisitor complete\n";
			llvm::outs() << std::format("totalCount {}\n", visitor.totalCount);
			llvm::outs() << std::format("attrCount {}\n", visitor.attrCount);
			llvm::outs() << std::format("declCount {}\n", visitor.declCount);
			llvm::outs() << std::format("stmtCount {}\n", visitor.stmtCount);
			llvm::outs() << std::format("typeCount {}\n", visitor.typeCount);
			llvm::outs() << std::format("typeLocCount {}\n",
			  visitor.typeLocCount);
		}
#endif
		AstDumper dumper(sourceManager, langOpts, llvm::outs(),
		  clEnableLastChild, clFlushLeft);
		//dumper.setOutput(nullptr);
		dumper.setLogLevel(clVerbose ? 1 : 0);
		llvm::outs() << "AstDumper starting\n";
		dumper.Visit(tuDecl);
		llvm::outs() << "AstDumper complete\n";
		AstDumper::Stats dumperStats;
		dumper.getStats(dumperStats);
		llvm::outs() << std::format("visitCount {}\n", dumperStats.visitCount);
		llvm::outs() << std::format("attrCount {}\n", dumperStats.attrCount);
		llvm::outs() << std::format("declCount {}\n", dumperStats.declCount);
		llvm::outs() << std::format("stmtCount {}\n", dumperStats.stmtCount);
		llvm::outs() << std::format("typeCount {}\n", dumperStats.typeCount);
		llvm::outs() << std::format("typeLocCount {}\n",
		  dumperStats.typeLocCount);
	}
private:
	std::string fileName_;
	clang::CompilerInstance* compInstance_;
};

struct MyAstFrontendAction : public clang::ASTFrontendAction {
	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
	  clang::CompilerInstance& compInstance, clang::StringRef inFile)
	  override {
		return std::make_unique<MyAstConsumer>(compInstance,
		  std::string(inFile));
	}
};

int main(int argc, char** argv) {
	auto expectedOptionsParser = ct::CommonOptionsParser::create(argc,
	  const_cast<const char**>(argv), lc::getGeneralCategory());
	if (!expectedOptionsParser) {
		llvm::errs() << std::format("Unable to create option parser ({}).\n",
		  llvm::toString(expectedOptionsParser.takeError()));
		return 1;
	}
	ct::CommonOptionsParser& optionsParser = *expectedOptionsParser;
	ct::ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());
	int status = tool.run(
	  ct::newFrontendActionFactory<MyAstFrontendAction>().get());
	if (status) {llvm::errs() << "error occurred\n";}
	return !status ? 0 : 1;
}
