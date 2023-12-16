#include <format>
#include <string_view>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;

AST_MATCHER_P(clang::CXXRecordDecl, forEachVirtualBase,
  cam::internal::Matcher<clang::CXXBaseSpecifier>, innerMatcher) {
	bool matched = false;
	cam::internal::BoundNodesTreeBuilder result;
	const clang::CXXRecordDecl* def = Node.getDefinition();
	if (!def) {return false;}
	for (auto baseIter = def->vbases_begin(); baseIter != def->vbases_end();
	  ++baseIter) {
		cam::internal::BoundNodesTreeBuilder argBuilder(*Builder);
		if (innerMatcher.matches(*baseIter, Finder, &argBuilder)) {
			matched = true;
			result.addMatch(argBuilder);
		}
	}
	*Builder = std::move(result);
	return matched;
}

struct MyMatchCallback : public cam::MatchFinder::MatchCallback {
	void run(const cam::MatchFinder::MatchResult& result) override;
};

void MyMatchCallback::run(const cam::MatchFinder::MatchResult& result) {
	auto decl = result.Nodes.getNodeAs<clang::CXXRecordDecl>("decl");
	auto baseDecl = result.Nodes.getNodeAs<clang::CXXRecordDecl>(
	  "baseDecl");
	if (!decl || !baseDecl) {return;}
	llvm::outs() << std::format("{} has virtual base {}\n",
	  std::string_view(decl->getName()), std::string_view(baseDecl->getName()));
}

static llvm::cl::OptionCategory optionCategory("Tool options");

int main(int argc, const char **argv) {
	auto optParser = ct::CommonOptionsParser::create(argc, argv,
	  optionCategory);
	if (!optParser) {
		llvm::errs() << llvm::toString(optParser.takeError());
		return 1;
	}
	ct::ClangTool tool(optParser->getCompilations(),
	  optParser->getSourcePathList());
	cam::DeclarationMatcher matcher = [](){
		using namespace cam;
		return cxxRecordDecl(forEachVirtualBase(cxxBaseSpecifier(hasType(
		  qualType(hasDeclaration(cxxRecordDecl().bind("baseDecl")))))))
		  .bind("decl");
	}();
	MyMatchCallback matchCallback;
	cam::MatchFinder matchFinder;
	matchFinder.addMatcher(matcher, &matchCallback);
	return tool.run(ct::newFrontendActionFactory(&matchFinder).get());
}
