#include <cassert>
#include <format>
#include <map>
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;

static llvm::cl::OptionCategory optionCategory("Tool options");

template<class NodeType>
const NodeType* getParentOfStmt(clang::ASTContext& astContext,
  const clang::Stmt* stmt) {
	auto parents = astContext.getParents(*stmt);
	const clang::Stmt* curStmt = nullptr;
	const NodeType* parent = nullptr;
	for (auto&& node : parents) {
		if (auto p = node.get<NodeType>()) {
			assert(!parent);
			parent = p;
		}
	}
	return parent;
}

unsigned getForDepth(clang::ASTContext& astContext,
  const clang::Stmt* forStmt) {
	assert(llvm::isa<clang::ForStmt>(forStmt) ||
	  llvm::isa<clang::CXXForRangeStmt>(forStmt));
	unsigned count = 1;
	const clang::Stmt* curStmt = forStmt;
	while ((curStmt = getParentOfStmt<clang::Stmt>(astContext, curStmt))) {
		if (llvm::isa<clang::ForStmt>(curStmt) ||
		  llvm::isa<clang::CXXForRangeStmt>(curStmt)) {++count;}
	}
	return count;
}

class MyMatchCallback : public cam::MatchFinder::MatchCallback {
public:
	void run(const cam::MatchFinder::MatchResult& result) final;
	void onStartOfTranslationUnit() final {funcTab_.clear();}
	void onEndOfTranslationUnit() final;
private:
	using FuncTab = std::map<const clang::FunctionDecl*, unsigned>;
	FuncTab funcTab_;
};

void MyMatchCallback::onEndOfTranslationUnit() {
	for (auto [funcDecl, maxForDepth] : funcTab_) {
		llvm::outs() << std::format("{} ... {}\n",
		  funcDecl->getQualifiedNameAsString(), maxForDepth);
	}
	funcTab_.clear();
}

void MyMatchCallback::run(const cam::MatchFinder::MatchResult& result) {
	const clang::SourceManager& sourceManager = *result.SourceManager;
	auto forStmt = result.Nodes.getNodeAs<clang::Stmt>("for");
	auto funcDecl = result.Nodes.getNodeAs<clang::FunctionDecl>("func");
	if (funcDecl && forStmt) {
		auto iter = funcTab_.find(funcDecl);
		if (iter == funcTab_.end()) {
			iter = funcTab_.insert(std::make_pair(funcDecl, 0)).first;
		}
		unsigned depth = getForDepth(*result.Context, forStmt);
		iter->second = std::max(iter->second, depth);
	}
}

cam::StatementMatcher getMatcher() {
	using namespace cam;
	auto f = anyOf(forStmt(), cxxForRangeStmt());
	return stmt(f, hasAncestor(functionDecl(isExpansionInMainFile()).bind(
	  "func")), unless(hasDescendant(stmt(f)))).bind("for");
}

struct MyAstConsumer : public clang::ASTConsumer {
	void HandleTranslationUnit(clang::ASTContext& astContext) final {
		MyMatchCallback matchCallback;
		cam::StatementMatcher matcher = getMatcher();
		cam::MatchFinder matchFinder;
		matchFinder.addMatcher(matcher, &matchCallback);
		matchFinder.matchAST(astContext);
	}
};

struct MyFrontendAction : public clang::ASTFrontendAction {
	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
	  clang::CompilerInstance&, clang::StringRef fileName) final {
		llvm::outs() << std::format("PROCESSING SOURCE FILE {}\n",
		  std::string(fileName));
		return std::unique_ptr<clang::ASTConsumer>{new MyAstConsumer};
	}
};

int main(int argc, const char **argv) {
	auto expectedParser = ct::CommonOptionsParser::create(argc, argv,
	  optionCategory);
	if (!expectedParser) {
		llvm::errs() << llvm::toString(expectedParser.takeError());
		return 1;
	}
	ct::CommonOptionsParser& optionsParser = expectedParser.get();
	ct::ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());
	int status =
	  tool.run(ct::newFrontendActionFactory<MyFrontendAction>().get());
	if (status) {llvm::errs() << "error detected\n";}
	return !status ? 0 : 1;
}
