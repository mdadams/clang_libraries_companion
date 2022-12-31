#include <format>
#include <stack>
#include <type_traits>
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

namespace ct = clang::tooling;

class MyAstVisitor : public clang::RecursiveASTVisitor<MyAstVisitor> {
public:
	using Base = clang::RecursiveASTVisitor<MyAstVisitor>;
	MyAstVisitor(clang::ASTContext& astContext) : astContext_(&astContext) {}
	bool shouldVisitImplicitCode() const {return true;}
	bool shouldVisitTemplateInstantiations() const {return true;}
	bool TraverseFunctionDecl(clang::FunctionDecl* funcDecl)
	  {return handleFunc<clang::FunctionDecl>(funcDecl);}
	bool TraverseCXXMethodDecl(clang::CXXMethodDecl* funcDecl)
	  {return handleFunc<clang::CXXMethodDecl>(funcDecl);}
	bool TraverseForStmt(clang::ForStmt* forStmt)
	  {return handleFor<clang::ForStmt>(forStmt);}
	bool TraverseCXXForRangeStmt(clang::CXXForRangeStmt* forStmt)
	  {return handleFor<clang::CXXForRangeStmt>(forStmt);}
private:
	struct StackEntry {
		const clang::FunctionDecl* funcDecl;
		unsigned forDepth;
		unsigned maxForDepth;
	};
	template<class NodeType> bool handleFunc(NodeType* funcDecl);
	template<class NodeType> bool handleFor(NodeType* forStmt);
	clang::ASTContext* astContext_;
	std::stack<StackEntry> stack_;
};

template<class NodeType> bool MyAstVisitor::handleFunc(NodeType* funcDecl) {
	const clang::SourceManager& sourceManager =
	  astContext_->getSourceManager();
	if (sourceManager.getFileID(funcDecl->getLocation()) !=
	  sourceManager.getMainFileID()) {return true;}
	stack_.push({funcDecl, 0, 0});
	bool result;
	if constexpr (std::is_same_v<NodeType, clang::CXXMethodDecl>)
	  {result = Base::TraverseCXXMethodDecl(funcDecl);}
	else {result = Base::TraverseFunctionDecl(funcDecl);}
	if (stack_.top().maxForDepth > 0) {
		llvm::outs() << std::format("{} ... {}\n",
		  stack_.top().funcDecl->getQualifiedNameAsString(),
		  stack_.top().maxForDepth);
	}
	stack_.pop();
	return result;
}

template<class NodeType> bool MyAstVisitor::handleFor(NodeType* forStmt) {
	if (stack_.empty()) {return true;}
	StackEntry& top = stack_.top();
	++top.forDepth;
	top.maxForDepth = std::max(top.maxForDepth, top.forDepth);
	bool result;
	if constexpr(std::is_same_v<NodeType, clang::CXXForRangeStmt>)
	  {result = Base::TraverseCXXForRangeStmt(forStmt);}
	else {result = Base::TraverseForStmt(forStmt);}
	--top.forDepth;
	return result;
}

struct MyAstConsumer : public clang::ASTConsumer {
	void HandleTranslationUnit(clang::ASTContext& astContext) final {
		MyAstVisitor visitor(astContext);
		visitor.TraverseDecl(astContext.getTranslationUnitDecl());
	}
};

struct MyFrontendAction : public clang::ASTFrontendAction {
	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
	  clang::CompilerInstance&, clang::StringRef fileName) final {
		llvm::outs() << std::format("PROCESSING SOURCE FILE {}\n", fileName);
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
