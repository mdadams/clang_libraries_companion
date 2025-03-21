#include <format>

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

#include "AstDumper.hpp"

namespace ct = clang::tooling;
namespace lc = llvm::cl;

static lc::OptionCategory toolOptions("Tool Options");
static lc::opt<int> clLogLevel("log-level", lc::init(0));
static bool clVisitTemplateInstantiations = true;
static lc::opt<bool> clDummy1("visit-template-instantiations",
  lc::callback([](const bool&){clVisitTemplateInstantiations = true;}));
static lc::opt<bool> clDummy2("no-visit-template-instantiations",
  lc::callback([](const bool&){clVisitTemplateInstantiations = false;}));

class MyAstConsumer : public clang::ASTConsumer {
public:
	MyAstConsumer(clang::CompilerInstance& compInstance,
	  const std::string& fileName) : compInstance_(&compInstance),
	  fileName_(fileName) {}
	void HandleTranslationUnit(clang::ASTContext& astContext) override {
		clang::SourceManager& sourceManager = compInstance_->getSourceManager();
		const clang::LangOptions& langOpts = compInstance_->getLangOpts();
		clang::TranslationUnitDecl* tuDecl =
		  astContext.getTranslationUnitDecl();
		AstDumper visitor(sourceManager, langOpts, &(llvm::outs()));
		visitor.setLogLevel(clLogLevel);
		visitor.doTemplateInstantiations(clVisitTemplateInstantiations);
		visitor.setPrefix(clLogLevel >= 1 ? "TREE: " : "");
		visitor.TraverseDecl(tuDecl);
		AstDumper::Stats dumperStats;
		visitor.getStats(dumperStats);
		llvm::outs() << std::format("visitCount {}\n", dumperStats.totalCount);
		llvm::outs() << std::format("attrCount {}\n", dumperStats.attrCount);
		llvm::outs() << std::format("declCount {}\n", dumperStats.declCount);
		llvm::outs() << std::format("stmtCount {}\n", dumperStats.stmtCount);
		llvm::outs() << std::format("typeCount {}\n", dumperStats.typeCount);
		llvm::outs() << std::format("typeLocCount {}\n", dumperStats.typeLocCount);
		llvm::outs() << std::format("conceptRefCount {}\n",
		  dumperStats.conceptRefCount);
		llvm::outs() << std::format("cxxBaseSpecCount {}\n",
		  dumperStats.cxxBaseSpecCount);
		llvm::outs() << std::format("ctorInitCount {}\n",
		  dumperStats.ctorInitCount);
		llvm::outs() << std::format("lambdaCaptureCount {}\n",
		  dumperStats.lambdaCaptureCount);
		llvm::outs() << std::format("nestedNameSpecCount {}\n",
		  dumperStats.nestedNameSpecCount);
		llvm::outs() << std::format("nestedNameSpecLocCount {}\n",
		  dumperStats.nestedNameSpecLocCount );
		llvm::outs() << std::format("tempArgCount {}\n",
		  dumperStats.tempArgCount);
		llvm::outs() << std::format("tempArgLocCount {}\n",
		  dumperStats.tempArgLocCount);
		llvm::outs() << std::format("tempNameCount {}\n",
		  dumperStats.tempNameCount );
		llvm::outs() << std::format("objcProtocolLocCount {}\n",
		  dumperStats.objcProtocolLocCount );
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
	  const_cast<const char**>(argv), toolOptions);
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
