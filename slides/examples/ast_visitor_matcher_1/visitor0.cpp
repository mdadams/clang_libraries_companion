#include <format>
#include <map>
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "utility.hpp"

namespace ct = clang::tooling;

const clang::Stmt* getTopLevelStmt(clang::ASTContext& astContext,
  const clang::Stmt* stmt) {
	const clang::Stmt* curStmt = stmt;
	for (;;) {
		const clang::Stmt* nextStmt = getParentOfStmt<clang::Stmt>(astContext,
		  curStmt);
		if (!nextStmt) {break;}
		curStmt = nextStmt;
	}
	return curStmt;
}

const clang::FunctionDecl* getContainingFuncDecl(clang::ASTContext& astContext,
  const clang::Stmt* stmt) {
	const clang::Stmt* topStmt = getTopLevelStmt(astContext, stmt);
	return getParentOfStmt<clang::FunctionDecl>(astContext, topStmt);
}

class MyAstVisitor : public clang::RecursiveASTVisitor<MyAstVisitor> {
public:
	using FuncTab = std::map<const clang::FunctionDecl*, unsigned>;
	MyAstVisitor(clang::ASTContext& astContext, FuncTab& funcTab) :
	  astContext_(&astContext), funcTab_(&funcTab) {}
	bool VisitForStmt(clang::ForStmt* forStmt)
	  {return handleForStatement(forStmt);}
	bool VisitCXXForRangeStmt(clang::CXXForRangeStmt* forStmt)
	  {return handleForStatement(forStmt);}
	bool shouldVisitImplicitCode() const {return true;}
private:
	bool handleForStatement(clang::Stmt* forStmt) {
		const clang::FunctionDecl* funcDecl =
		  getContainingFuncDecl(*astContext_, forStmt);
		assert(funcDecl);
		const clang::SourceManager& sourceManager =
		  astContext_->getSourceManager();
		if (sourceManager.getFileID(funcDecl->getLocation()) !=
		  sourceManager.getMainFileID()) {return true;}
		unsigned forDepth = getForDepth(*astContext_, forStmt);
		auto funcTabIter = funcTab_->find(funcDecl);
		if (funcTabIter == funcTab_->end()) {
			funcTabIter = funcTab_->insert(std::make_pair(funcDecl,
			  forDepth)).first;
		}
		funcTabIter->second = std::max(funcTabIter->second, forDepth);
		return true;
	}
	clang::ASTContext* astContext_;
	FuncTab* funcTab_;
};

class MyAstConsumer : public clang::ASTConsumer {
public:
	void HandleTranslationUnit(clang::ASTContext& astContext) final {
		MyAstVisitor visitor(astContext, funcTab_);
		visitor.TraverseDecl(astContext.getTranslationUnitDecl());
		for (auto [funcDecl, maxForDepth] : funcTab_) {
			llvm::outs() << std::format("{} ... {}\n",
			  funcDecl->getQualifiedNameAsString(), maxForDepth);
		}
	}
private:
	MyAstVisitor::FuncTab funcTab_;
};

struct MyFrontendAction : public clang::ASTFrontendAction {
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
