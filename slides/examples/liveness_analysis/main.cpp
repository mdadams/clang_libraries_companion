#include <format>
#include <string>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include "analyze.hpp"

namespace cam = clang::ast_matchers;
namespace ct = clang::tooling;
namespace lc = llvm::cl;

static lc::OptionCategory toolCategory("Tool Options");
static lc::opt<std::string> clFuncNamePattern("f", lc::cat(toolCategory),
  lc::init(".*"));
static lc::opt<bool> clPrintCfg("c", lc::cat(toolCategory), lc::init(false));

struct MyMatchCallback : public cam::MatchFinder::MatchCallback {
	virtual void run(const cam::MatchFinder::MatchResult& result) final {
		if (auto funcDecl =
		  result.Nodes.getNodeAs<clang::FunctionDecl>("func")) {
			clang::ASTContext *astContext = result.Context;
			clang::Stmt *funcBody = funcDecl->getBody();
			if (!funcBody) {return;}
			llvm::outs() << std::format("FUNCTION: {}\n",
			  funcDecl->getQualifiedNameAsString());
			analyzeFunc(*astContext, funcDecl, clPrintCfg);
		}
	}
};

cam::DeclarationMatcher getFuncMatcher(const std::string& namePattern)
  {return cam::functionDecl(cam::matchesName(namePattern)).bind("func");}

int main(int argc, const char **argv) {
	llvm::Expected<ct::CommonOptionsParser> expOptionsParser =
	ct::CommonOptionsParser::create(argc, argv, toolCategory);
	if (!expOptionsParser) {
		llvm::errs() << llvm::toString(expOptionsParser.takeError());
		return 1;
	}
	ct::CommonOptionsParser& optionsParser = *expOptionsParser;
	ct::ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());
	cam::DeclarationMatcher funcMatcher = getFuncMatcher(clFuncNamePattern);
	MyMatchCallback matchCallback;
	cam::MatchFinder finder;
	finder.addMatcher(funcMatcher, &matchCallback);
	int status = tool.run(ct::newFrontendActionFactory(&finder).get());
	if (status) {llvm::errs() << "error occurred\n";}
	return !status ? 0 : 1;
}
