#include <format>
#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <clang/AST/ASTDumper.h>
#include <clang/AST/JSONNodeDumper.h>

namespace ct = clang::tooling;
namespace lc = llvm::cl;

static lc::OptionCategory toolOptions("Tool Options");
static lc::opt<bool> textFormat("text");

class MyAstConsumer : public clang::ASTConsumer {
public:
	MyAstConsumer(clang::CompilerInstance& compInstance,
	  const std::string& fileName) : compInstance_(&compInstance),
	  fileName_(fileName) {}
	void HandleTranslationUnit(clang::ASTContext& astContext) override {
		if (textFormat) {
			llvm::outs() << std::format("source file: {}\n", fileName_);
			clang::ASTDumper dumper(llvm::outs(), astContext, false);
			dumper.Visit(astContext.getTranslationUnitDecl());
		} else {
			clang::SourceManager& sourceManager = compInstance_->getSourceManager();
			clang::JSONDumper dumper(llvm::outs(), sourceManager, astContext,
			  astContext.getPrintingPolicy(),
			  &astContext.getCommentCommandTraits());
			dumper.Visit(astContext.getTranslationUnitDecl());
		}
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
