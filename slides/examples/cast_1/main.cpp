#include <format>
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;

std::string locationToString(const clang::SourceManager& sourceManager,
  clang::SourceLocation sourceLoc) {
	return std::format("{}:{}({})",
	  std::string(sourceManager.getFilename(sourceLoc)),
	  sourceManager.getSpellingLineNumber(sourceLoc),
	  sourceManager.getSpellingColumnNumber(sourceLoc));
}

std::string getCastName(const clang::ExplicitCastExpr* castExpr) {
	if (llvm::dyn_cast<clang::CStyleCastExpr>(castExpr)) {
		return "C-style-cast";
	} else if (llvm::dyn_cast<clang::CXXFunctionalCastExpr>(castExpr)) {
		return "functional-cast";
	} else if (llvm::dyn_cast<clang::BuiltinBitCastExpr>(castExpr)) {
		return "bit-cast";
	} else if (auto namedCastExpr =
	  llvm::dyn_cast<clang::CXXNamedCastExpr>(castExpr)) {
		return namedCastExpr->getCastName();
	} else {return "unknown";}
}

struct MyMatchCallback : public cam::MatchFinder::MatchCallback {
	void run(const cam::MatchFinder::MatchResult& result) override;
};

void MyMatchCallback::run(const cam::MatchFinder::MatchResult& result) {
	const clang::SourceManager& sourceManager = *result.SourceManager;
	const clang::ASTContext& astContext = *result.Context;
	auto castExpr = result.Nodes.getNodeAs<clang::ExplicitCastExpr>("c");
	clang::QualType targetQualType = castExpr->getType();
	clang::SourceLocation loc = castExpr->getExprLoc();
	const clang::Expr* sourceExpr = castExpr->getSubExpr();
	clang::QualType sourceQualType = sourceExpr->getType();
	clang::PrintingPolicy pp(astContext.getLangOpts());
	pp.PrintCanonicalTypes = 1;
	llvm::outs() << std::format(
	  "location: {}\ncast: {}\nfrom type: {}\nto type: {}\n\n",
	  locationToString(sourceManager, loc), getCastName(castExpr),
	  sourceQualType.getAsString(pp), targetQualType.getAsString(pp)
	  );
}

static llvm::cl::OptionCategory optionCategory("Tool options");

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
	MyMatchCallback matchCallback;
	cam::MatchFinder matchFinder;
	matchFinder.addMatcher(cam::explicitCastExpr().bind("c"),
	  &matchCallback);
	return !tool.run(ct::newFrontendActionFactory(&matchFinder).get()) ? 0 : 1;
}
