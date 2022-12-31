#include <format>
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

namespace ct = clang::tooling;

class MyAstConsumer : public clang::ASTConsumer {
public:
	MyAstConsumer(const std::string& fileName) : fileName_(fileName) {}
	void HandleTranslationUnit(clang::ASTContext& astContext) override {
		llvm::outs() << std::format("input file: {}\nAST size: {}\n",
		  fileName_, astContext.getASTAllocatedMemory());
	}
private:
	std::string fileName_;
};

struct MyAstFrontendAction : public clang::ASTFrontendAction {
	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
	  clang::CompilerInstance&, clang::StringRef inFile) override {
		return std::make_unique<MyAstConsumer>(std::string(inFile));
	}
};

static llvm::cl::OptionCategory toolOptions("Tool Options");

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
