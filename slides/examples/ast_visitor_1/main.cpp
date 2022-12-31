#include <format>
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/CommandLine.h"

namespace ct = clang::tooling;

class MyAstVisitor : public clang::RecursiveASTVisitor<MyAstVisitor> {
public:
	MyAstVisitor(clang::ASTContext& astContext) : astContext_(&astContext) {}
	bool VisitFunctionDecl(clang::FunctionDecl* funcDecl) {
		const auto& fileId = astContext_->getSourceManager().getFileID(
		  funcDecl->getLocation());
		if (fileId == astContext_->getSourceManager().getMainFileID()) {
			llvm::outs() << std::format("{}\n",
			  funcDecl->getQualifiedNameAsString());
		}
		return true;
	}
private:
	clang::ASTContext* astContext_;
};

class MyAstConsumer : public clang::ASTConsumer {
public:
	void HandleTranslationUnit(clang::ASTContext& astContext) final {
		clang::TranslationUnitDecl* tuDecl =
		  astContext.getTranslationUnitDecl();
		MyAstVisitor visitor(astContext);
		visitor.TraverseDecl(tuDecl);
	}
};

class MyFrontendAction : public clang::ASTFrontendAction {
public:
	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
	  clang::CompilerInstance&, clang::StringRef) final {
		return std::unique_ptr<clang::ASTConsumer>{new MyAstConsumer};
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
	  ct::newFrontendActionFactory<MyFrontendAction>().get());
	if (status) {llvm::errs() << "error detected\n";}
	return !status ? 0 : 1;
}
