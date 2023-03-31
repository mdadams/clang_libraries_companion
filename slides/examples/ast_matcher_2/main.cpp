#include <format>
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "utilities2.hpp"

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;

static llvm::cl::OptionCategory optionCategory("Tool options");
static llvm::cl::opt<int> clMatcherId("m", llvm::cl::desc("Matcher ID"),
  llvm::cl::value_desc("matcher_id"), llvm::cl::cat(optionCategory),
  llvm::cl::init(0));
static llvm::cl::opt<bool> clAllNodes("a", llvm::cl::desc("all nodes"),
  llvm::cl::cat(optionCategory), llvm::cl::init(false));

AST_MATCHER(clang::CXXMethodDecl, isSpecialMember) {
	if (auto p = llvm::dyn_cast<clang::CXXConstructorDecl>(&Node)) {
		return p->isDefaultConstructor() || p->isCopyConstructor() ||
		  p->isMoveConstructor();
	} else if (auto p = llvm::dyn_cast<clang::CXXDestructorDecl>(&Node)) {
		return true;
	} else {
		return Node.isCopyAssignmentOperator() ||
		  Node.isMoveAssignmentOperator();
	}
}

AST_MATCHER_P(clang::CXXMethodDecl, paramCountAtLeast, unsigned, threshold) {
	return Node.param_size() >= threshold;
}

AST_MATCHER_P2(clang::NamedDecl, nameLengthBetween, unsigned, low, unsigned,
  high) {
	return Node.getIdentifier() && Node.getName().size() >= low &&
	  Node.getName().size() <= high;
}

cam::DeclarationMatcher getMatcher(int id) {
	using namespace cam;
	switch (id) {
	default:
	case 0:
		return cxxMethodDecl(isDefinition(), isSpecialMember()).bind("x");
	case 1:
		return cxxMethodDecl(paramCountAtLeast(4)).bind("x");
	case 2:
		return namedDecl(nameLengthBetween(3, 4)).bind("x");
	}
}

class MyMatchCallback : public cam::MatchFinder::MatchCallback {
public:
	MyMatchCallback() : count_(0) {}
	void run(const cam::MatchFinder::MatchResult& result) override {
		const clang::SourceManager& sourceManager = *result.SourceManager;
		clang::SourceRange sourceRange;
		std::string nodeType;
		if (auto p = result.Nodes.getNodeAs<clang::CXXMethodDecl>("x")) {
			nodeType = "CXXMethodDecl";
			sourceRange = p->getSourceRange();
		} else if (auto p = result.Nodes.getNodeAs<clang::FunctionDecl>("x")) {
			nodeType = "FunctionDecl";
			sourceRange = p->getSourceRange();
		}
		if (sourceRange.isValid()) {
			llvm::outs() << std::format("found matching {} at {}\n", nodeType,
			  locationToString(sourceManager, sourceRange.getBegin(), true));
			sourceRange.setBegin(sourceManager.getSpellingLoc(sourceRange.getBegin()));
			sourceRange.setEnd(sourceManager.getSpellingLoc(sourceRange.getEnd()));
			sourceRange.setEnd(getEndOfToken(sourceManager,
			  sourceRange.getEnd()));
			if (sourceRange.isValid()) {
				llvm::outs() << getSourceTextRaw(sourceManager, sourceRange) << '\n';
			}
		}
		++count_;
	}
	unsigned getCount() const {return count_;}
private:
	unsigned count_;
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
	cam::DeclarationMatcher matcher = getMatcher(clMatcherId);
	if (!clAllNodes) {
		matcher = cam::traverse(clang::TK_IgnoreUnlessSpelledInSource,
		  matcher);
	}
	MyMatchCallback matchCallback;
	cam::MatchFinder matchFinder;
	matchFinder.addMatcher(matcher, &matchCallback);
	int status = tool.run(ct::newFrontendActionFactory(&matchFinder).get());
	llvm::outs() << std::format("number of matches: {}\n",
	  matchCallback.getCount());
	return !status ? 0 : 1;
}
