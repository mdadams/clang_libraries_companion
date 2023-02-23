#include <format>
#include <string>
#include <map>
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

std::string declTypeToString(const clang::Decl* decl) {
	if (llvm::isa<clang::FunctionDecl>(decl)) {return "function";}
	else if (llvm::isa<clang::VarDecl>(decl)) {return "variable";}
	else if (llvm::isa<clang::FieldDecl>(decl)) {return "field";}
	else if (llvm::isa<clang::CXXRecordDecl>(decl)) {return "class";}
	else {return "other";}
}

std::string attrSyntaxToString(clang::Attr::Syntax syntax) {
	const std::map<clang::Attr::Syntax, std::string> lut{
		{clang::Attr::Syntax::AS_GNU, "AS_GNU"},
		{clang::Attr::Syntax::AS_CXX11, "AS_CXX11"},
		{clang::Attr::Syntax::AS_Keyword, "AS_Keyword"},
		{clang::Attr::Syntax::AS_Pragma, "AS_Pragma"}
	};
	auto i = lut.find(syntax);
	return (i != lut.end()) ? i->second : "unknown";
}

struct MyMatchCallback : public cam::MatchFinder::MatchCallback {
	void run(const cam::MatchFinder::MatchResult& result) override;
};

void MyMatchCallback::run(const cam::MatchFinder::MatchResult& result) {
	const clang::SourceManager& sourceManager = *result.SourceManager;
	auto decl = result.Nodes.getNodeAs<clang::NamedDecl>("d");
	if (!decl) {return;}
	llvm::outs() << std::format("{} {}\n", declTypeToString(decl),
	  decl->getIdentifier() ? decl->getName() : "---");
	for (auto attrIter = decl->attr_begin(); attrIter != decl->attr_end();
	  ++attrIter) {
		const clang::Attr* attr = *attrIter;
		clang::SourceLocation loc = attr->getLoc();
		llvm::outs() << std::format("  attribute {} {} {}\n",
		  attr->getAttrName() ? attr->getNormalizedFullName() : "---",
		  attrSyntaxToString(attr->getSyntax()),
		  locationToString(sourceManager, loc));
	}
}

int main(int argc, const char **argv) {
	static llvm::cl::OptionCategory optionCategory("Tool options");
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
	matchFinder.addMatcher(cam::namedDecl(cam::has(cam::attr())).bind("d"),
	  &matchCallback);
	return !tool.run(ct::newFrontendActionFactory(&matchFinder).get()) ? 0 : 1;
}
