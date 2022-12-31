#include <format>
#include "clang/Analysis/CFG.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;

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
	numEdges -= 2;
	return numEdges - numNodes + (2 * 1);
}

struct MyMatchCallback : public cam::MatchFinder::MatchCallback {
	using MatchResult = cam::MatchFinder::MatchResult;
	void run(const MatchResult& result) override {
		const auto* function =
		  result.Nodes.getNodeAs<clang::FunctionDecl>("f");
		std::string s = function->getQualifiedNameAsString();
		int complexity = cyclomaticComplexity(*function,
		  *result.Context);
		if (complexity >= 0 && complexity >= thresholdOption) {
			llvm::outs() << std::format("{} {}\n", s, complexity);
		}
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
	auto matcher =
	  cam::functionDecl(cam::isExpansionInMainFile()).bind("f");
	MyMatchCallback matchCallback;
	cam::MatchFinder matchFinder;
	matchFinder.addMatcher(matcher, &matchCallback);
	ct::ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());
	auto status =
	  tool.run(ct::newFrontendActionFactory(&matchFinder).get());
    if (status) {llvm::errs() << "error detected\n";}
	return !status ? 0 : 1;
}
