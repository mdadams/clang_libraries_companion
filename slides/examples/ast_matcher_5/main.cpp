#include <format>
#include <string>
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/Dynamic/VariantValue.h"
#include "clang/AST/Type.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;
namespace lc = llvm::cl;

cam::dynamic::VariantMatcher getMatcher(int id) {
	using namespace cam;
	switch (id) {
	default:
	case 0:
		return dynamic::VariantMatcher::SingleMatcher(qualType().bind("qt"));
	case 1:
		return dynamic::VariantMatcher::SingleMatcher(varDecl().bind("v"));
	case 2:
		return dynamic::VariantMatcher::SingleMatcher(
		  callExpr(callee(functionDecl().bind("f"))));
	}
}

cam::dynamic::VariantMatcher traverse(clang::TraversalKind kind,
  cam::dynamic::VariantMatcher matcher) {
	using namespace cam;
	if (matcher.hasTypedMatcher<clang::Decl>()) {
		return dynamic::VariantMatcher::SingleMatcher(traverse(kind,
		  matcher.getTypedMatcher<clang::Decl>()));
	} else if (matcher.hasTypedMatcher<clang::Stmt>()) {
		return dynamic::VariantMatcher::SingleMatcher(traverse(kind,
		  matcher.getTypedMatcher<clang::Stmt>()));
	} else if (matcher.hasTypedMatcher<clang::QualType>()) {
		return dynamic::VariantMatcher::SingleMatcher(traverse(kind,
		  matcher.getTypedMatcher<clang::QualType>()));
	} else {std::abort();}
	return matcher;
}

struct MyMatchCallback : public cam::MatchFinder::MatchCallback {
	MyMatchCallback() : count(0) {}
	void run(const cam::MatchFinder::MatchResult& result) override;
	unsigned count;
};

void MyMatchCallback::run(const cam::MatchFinder::MatchResult& result) {
	llvm::outs() << std::format("MATCH {}:\n", count);
	if (auto qualTypePtr = result.Nodes.getNodeAs<clang::QualType>("qt")) {
		llvm::outs() << "dump for QualType:\n";
		qualTypePtr->dump();
	} else if (auto varDecl =
	  result.Nodes.getNodeAs<clang::VarDecl>("v")) {
		std::string s(varDecl->getQualifiedNameAsString());
		llvm::outs() << std::format("dump for VarDecl {}:\n",
		  !s.empty() ? s : "[unnamed]");
		varDecl->dump();
	} else if (auto funcDecl =
	  result.Nodes.getNodeAs<clang::FunctionDecl>("f")) {
		llvm::outs() << std::format("dump for FunctionDecl {}:\n",
		  funcDecl->getQualifiedNameAsString());
		funcDecl->dump();
	}
	++count;
}

static lc::OptionCategory optionCategory("Tool options");
static lc::list<int> clMatcherIds("m", lc::desc("Matcher ID"),
  lc::cat(optionCategory), lc::ZeroOrMore);
static lc::opt<bool> clAsIs("i", lc::desc("Implicit nodes"),
  lc::cat(optionCategory));

int main(int argc, const char **argv) {
	auto optParser = ct::CommonOptionsParser::create(argc, argv,
	  optionCategory);
	if (!optParser) {
		llvm::errs() << llvm::toString(optParser.takeError());
		return 1;
	}
	ct::ClangTool tool(optParser->getCompilations(),
	  optParser->getSourcePathList());
	MyMatchCallback matchCallback;
	cam::MatchFinder matchFinder;
	for (auto id : clMatcherIds) {
		matchFinder.addDynamicMatcher(*traverse(
		  clAsIs ? clang::TK_AsIs : clang::TK_IgnoreUnlessSpelledInSource,
		  getMatcher(id)).getSingleMatcher(), &matchCallback);
	}
	int status = tool.run(ct::newFrontendActionFactory(&matchFinder).get());
	llvm::outs() << std::format("number of matches: {}\n",
	  matchCallback.count);
	return !status ? 0 : 1;
}
