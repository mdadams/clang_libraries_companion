#include <format>
#include "clang/Analysis/CFG.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

namespace ct = clang::tooling;

static llvm::cl::OptionCategory toolCategory("Tool Options");
static llvm::cl::opt<unsigned int> thresholdOption("t",
  llvm::cl::init(0), llvm::cl::desc("Set complexity threshold."),
  llvm::cl::cat(toolCategory));

int cyclomaticComplexity(const clang::FunctionDecl& funcDecl,
  clang::ASTContext& astContext) {
	const auto cfg = clang::CFG::buildCFG(&funcDecl, funcDecl.getBody(),
	  &astContext, clang::CFG::BuildOptions());
	if (!cfg) {return -1;}
	const int numNodes = cfg->size() - 2;
	int numEdges = 0;
	for (const auto* block : *cfg) {numEdges += block->succ_size();}
	numEdges -= 2; // adjust for entry and exit blocks
	return numEdges - numNodes + (2 * 1); // E - V + 2 * P
}

class MyAstVisitor : public clang::RecursiveASTVisitor<MyAstVisitor> {
public:
	MyAstVisitor(clang::ASTContext& astContext) : astContext_(&astContext) {}
	bool VisitFunctionDecl(clang::FunctionDecl* funcDecl) {
		const auto& fileId = astContext_->getSourceManager().getFileID(
		  funcDecl->getLocation());
		if (fileId == astContext_->getSourceManager().getMainFileID()) {
			std::string s = funcDecl->getQualifiedNameAsString();
			int complexity = cyclomaticComplexity(*funcDecl,
			  *astContext_);
			if (complexity >= 0 && complexity >= thresholdOption) {
				llvm::outs() << std::format("{} {}\n", s, complexity);
			}
		}
		return true;
	}
	bool shouldVisitTemplateInstantiations() const {return true;}
private:
	clang::ASTContext* astContext_;
};

struct MyAstConsumer : public clang::ASTConsumer {
	void HandleTranslationUnit(clang::ASTContext& astContext) final {
		clang::TranslationUnitDecl* tuDecl =
		  astContext.getTranslationUnitDecl();
		MyAstVisitor astVisitor(astContext);
		astVisitor.TraverseDecl(tuDecl);
	}
};

struct MyFrontendAction : public clang::ASTFrontendAction {
	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
	  clang::CompilerInstance& compilerInstance, llvm::StringRef) override {
		return std::make_unique<MyAstConsumer>();
	}
};

int main(int argc, char** argv) {
	auto expectedOptionsParser = ct::CommonOptionsParser::create(argc,
	const_cast<const char**>(argv), toolCategory);
	if (!expectedOptionsParser) {
		llvm::errs() << llvm::toString(expectedOptionsParser.takeError());
		return 1;
	}
	ct::CommonOptionsParser& optionsParser = *expectedOptionsParser;
	ct::ClangTool tool(optionsParser.getCompilations(),
	optionsParser.getSourcePathList());
	auto status =
	  tool.run(ct::newFrontendActionFactory<MyFrontendAction>().get());
    if (status) {llvm::errs() << "error detected\n";}
	return !status ? 0 : 1;
}
