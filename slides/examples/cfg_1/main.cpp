#include <format>
#include <map>
#include <string>
#include "clang/Analysis/CFG.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

namespace lc = llvm::cl;
namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;

static lc::OptionCategory toolCategory("Tool Options");
static lc::opt<std::string> clFuncName("f", lc::cat(toolCategory));

std::string toString(clang::CFGElement::Kind kind) {
	const std::map<clang::CFGElement::Kind, std::string> lut{
	  {clang::CFGElement::Kind::Statement, "statement"},
	  {clang::CFGElement::Kind::Constructor, "constructor"},
	  {clang::CFGElement::Kind::CXXRecordTypedCall, "recordTypedCall"},
	};
	auto i = lut.find(kind);
	return std::format("{}", (i != lut.end() ? i->second : "unknown"));
}

void printBlock(llvm::raw_ostream& out, const clang::CFG& cfg,
  const clang::CFGBlock& block) {
	out << std::format("block: {}", block.BlockID);
	if (&block == &cfg.getEntry()) {out << " (entry)";}
	if (&block == &cfg.getExit()) {out << " (exit)";}
	if (block.hasNoReturnElement()) {out << " (noreturn)";}
	out << '\n';
	if (block.succ_size()) {
		out << "successors:";
		for (auto succBlockIter = block.succ_begin(); succBlockIter !=
		  block.succ_end(); ++succBlockIter) {
			out << std::format(" {}", (*succBlockIter) ? std::format("{}",
			  (*succBlockIter)->BlockID) : "invalid");
		}
		out << '\n';
	}
	for (auto elemIter = block.begin(); elemIter != block.end(); ++elemIter) {
		out << std::format("{}: ", toString(elemIter->getKind()));
		elemIter->dumpToStream(out);
	}
}

void processFunc(const clang::FunctionDecl& funcDecl, clang::ASTContext&
  astContext) {
	llvm::outs() << std::format("FUNCTION: {}\n",
	  funcDecl.getQualifiedNameAsString());
	const auto cfg = clang::CFG::buildCFG(&funcDecl, funcDecl.getBody(),
	  &astContext, clang::CFG::BuildOptions());
	if (!cfg) {return;}
	for (auto blockIter = cfg->nodes_begin(); blockIter != cfg->nodes_end();
	  ++blockIter) {printBlock(llvm::outs(), *cfg, **blockIter);}
}

cam::DeclarationMatcher getFuncMatcher(const std::string& name) {
	return (name.size() ? cam::functionDecl(cam::hasName(name)) : 
	  cam::functionDecl()).bind("func");
}

struct MyMatchCallback : public cam::MatchFinder::MatchCallback {
	virtual void run(const cam::MatchFinder::MatchResult& result) final {
		if (const auto* funcDecl =
		  result.Nodes.getNodeAs<clang::FunctionDecl>("func")) {
			if (const clang::Stmt *funcBody = funcDecl->getBody())
			  {processFunc(*funcDecl, *result.Context);}
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
	ct::ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());
	cam::DeclarationMatcher funcMatcher = getFuncMatcher(clFuncName);
	MyMatchCallback matchCallback;
	cam::MatchFinder finder;
	finder.addMatcher(funcMatcher, &matchCallback);
	int status = tool.run(ct::newFrontendActionFactory(&finder).get());
	if (status) {llvm::errs() << "error occurred\n";}
	return !status ? 0 : 1;
}
