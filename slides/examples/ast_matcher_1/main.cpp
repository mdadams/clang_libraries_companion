#include <format>
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "utilities.hpp"

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;

clang::SourceLocation getLineStart(const clang::SourceManager& sourceManager,
  clang::SourceLocation loc) {
	return sourceManager.translateLineCol(sourceManager.getFileID(loc),
	  sourceManager.getSpellingLineNumber(loc), 1);
}

clang::SourceLocation getLineEnd(const clang::SourceManager& sourceManager,
  clang::SourceLocation loc) {
	return sourceManager.translateLineCol(sourceManager.getFileID(loc),
	  sourceManager.getSpellingLineNumber(loc), ~0);
}

struct MyMatchCallback : public cam::MatchFinder::MatchCallback {
	void run(const cam::MatchFinder::MatchResult& result) override {
		clang::SourceManager& sourceManager = *result.SourceManager;
		if (auto p = result.Nodes.getNodeAs<clang::CallExpr>("call")) {
			clang::SourceLocation startLoc = p->getBeginLoc();
			clang::SourceLocation endLoc = p->getEndLoc();
			llvm::outs() << std::format("match at {}:\n", rangeToString(
			  sourceManager, clang::SourceRange(startLoc, endLoc)));
			clang::SourceLocation lineStartLoc = getLineStart(sourceManager,
			  startLoc);
			clang::SourceLocation lineEndLoc = getLineEnd(sourceManager,
			  endLoc);
			unsigned int startLineNo = sourceManager.getSpellingLineNumber(
			  lineStartLoc);
			std::string text = getSourceText(sourceManager,
			  clang::SourceRange(lineStartLoc, lineEndLoc));
			llvm::outs() << addLineNumbers(text, startLineNo) << "\n";
		}
	}
};

cam::StatementMatcher getMatcher(const std::string& funcName) {
	using namespace cam;
	return callExpr(callee(functionDecl(hasName(funcName)))).bind("call");
}

static llvm::cl::OptionCategory optionCategory("Tool options");
static llvm::cl::opt<std::string> clFuncName(
  "f", llvm::cl::desc("Function name"), llvm::cl::value_desc("function_name"),
  llvm::cl::cat(optionCategory), llvm::cl::Required);
;

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
	cam::StatementMatcher matcher = getMatcher(clFuncName);
	cam::MatchFinder matchFinder;
	matchFinder.addMatcher(matcher, &matchCallback);
	return tool.run(ct::newFrontendActionFactory(&matchFinder).get());
}
