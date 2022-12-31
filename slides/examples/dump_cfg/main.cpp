#include <format>
#include <string>
#include "clang/Analysis/CFG.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

namespace cam = clang::ast_matchers;
namespace ct = clang::tooling;
namespace lc = llvm::cl;

static lc::OptionCategory toolCategory("Tool Options");
static lc::opt<std::string> clFuncNamePattern("f", lc::cat(toolCategory),
  lc::init(".*"));
static lc::opt<bool> clUseColor("c", lc::cat(toolCategory), lc::init(false));

cam::DeclarationMatcher getFuncMatcher(const std::string& namePattern)
  {return cam::functionDecl(cam::matchesName(namePattern)).bind("func");}

struct MyMatchCallback : public cam::MatchFinder::MatchCallback {
	virtual void run(const cam::MatchFinder::MatchResult& result) final {
		if (const auto* funcDecl =
		  result.Nodes.getNodeAs<clang::FunctionDecl>("func")) {
			clang::ASTContext *astContext = result.Context;
			clang::Stmt *funcBody = funcDecl->getBody();
			if (!funcBody) {return;}
			llvm::outs() << std::format("FUNCTION: {}\n",
			  funcDecl->getQualifiedNameAsString());
			std::unique_ptr<clang::CFG> cfg = clang::CFG::buildCFG(
			  funcDecl, funcBody, astContext, clang::CFG::BuildOptions());
			if (!cfg) {
				llvm::outs() << "unable to generate CFG\n";
				return;
			}
			auto langOpts = astContext->getLangOpts();
			cfg->print(llvm::outs(), langOpts, clUseColor);
		}
	}
};

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
