#include <format>
#include <string>
#include "clang/AST/Mangle.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/Dynamic/VariantValue.h"
#include "clang/AST/Type.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/Support/CommandLine.h"

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;
namespace lc = llvm::cl;

static lc::OptionCategory optionCategory("Tool options");
static lc::list<int> clMatcherIds("m", lc::desc("Matcher ID"),
  lc::cat(optionCategory), lc::ZeroOrMore);
static lc::opt<bool> clVerbose("v", lc::desc("Matcher ID"),
  lc::cat(optionCategory), lc::ZeroOrMore);
static const std::vector<int> defaultMatcherIds{0, 1, 2};

/****************************************************************************\
\****************************************************************************/

std::pair<std::string, bool> getSourceText(const clang::ASTContext&
  astContext, clang::SourceRange sourceRange, const clang::LangOptions*
  langOptions = nullptr)
{
	bool invalid = true;
	std::string buffer;
	if (sourceRange.isValid()) {
		buffer = clang::Lexer::getSourceText(
		  clang::CharSourceRange::getTokenRange(sourceRange),
		  astContext.getSourceManager(),
		  langOptions ? *langOptions : astContext.getLangOpts(), &invalid);
	}
	return {buffer, !invalid};
}

std::string expLocToString(const clang::SourceManager& sourceManager,
  clang::SourceLocation sourceLoc)
{
	return std::format("{}:{}:{}",
	  std::string(sourceManager.getFilename(sourceLoc)),
	  sourceManager.getExpansionLineNumber(sourceLoc),
	  sourceManager.getExpansionColumnNumber(sourceLoc));
}

/****************************************************************************\
\****************************************************************************/

std::string getMangledName(clang::MangleContext& mangleContext,
  clang::QualType qualType)
{
	assert(!qualType.isNull() && !qualType->isDependentType());
	std::string mangledName;
	llvm::raw_string_ostream mangledOut(mangledName);
	mangleContext.mangleTypeName(qualType, mangledOut);
	return mangledName;
}

std::string getMangledName(clang::MangleContext& mangleContext,
  clang::GlobalDecl decl)
{
	std::string mangledName;
	llvm::raw_string_ostream mangledOut(mangledName);
	mangleContext.mangleName(decl, mangledOut);
	return mangledName;
}

/****************************************************************************\
\****************************************************************************/

std::string getDemangledName(const std::string& mangledName)
{
	std::size_t size = 0;
	int status;
	char* result = llvm::itaniumDemangle(mangledName.c_str(), nullptr, &size,
	  &status);
	if (status != llvm::demangle_success) {
		return "";
	}
	std::string demangledName(result);
	std::free(result);
	return demangledName;
}

std::string demanglerGetFunctionBaseName(llvm::ItaniumPartialDemangler&
  demangler)
{
	std::size_t size = 0;
	char* p = demangler.getFunctionBaseName(nullptr, &size);
	std::string result(p ? p : "");
	std::free(p);
	return result;
}

std::string demanglerGetFunctionDeclContextName(llvm::ItaniumPartialDemangler&
  demangler)
{
	std::size_t size = 0;
	char* p = demangler.getFunctionDeclContextName(nullptr, &size);
	std::string result(p ? p : "");
	std::free(p);
	return result;
}

std::string demanglerGetFunctionName(llvm::ItaniumPartialDemangler&
  demangler)
{
	std::size_t size = 0;
	char* p = demangler.getFunctionName(nullptr, &size);
	std::string result(p ? p : "");
	std::free(p);
	return result;
}

std::string demanglerGetFunctionParameters(llvm::ItaniumPartialDemangler&
  demangler)
{
	std::size_t size = 0;
	char* p = demangler.getFunctionParameters(nullptr, &size);
	std::string result(p ? p : "");
	std::free(p);
	return result;
}

std::string demanglerGetFunctionReturnType(llvm::ItaniumPartialDemangler&
  demangler)
{
	std::size_t size = 0;
	char* p = demangler.getFunctionReturnType(nullptr, &size);
	std::string result(p ? p : "");
	std::free(p);
	return result;
}

/****************************************************************************\
\****************************************************************************/

cam::dynamic::VariantMatcher getMatcher(int id)
{
	using namespace cam;
	switch (id) {
	default:
	case 0:
		return dynamic::VariantMatcher::SingleMatcher(
		  qualType().bind("type"));
	case 1:
		return dynamic::VariantMatcher::SingleMatcher(varDecl().bind("var"));
	case 2:
		return dynamic::VariantMatcher::SingleMatcher(
		  functionDecl().bind("func"));
	case 3:
		return dynamic::VariantMatcher::SingleMatcher(
		  cxxConstructorDecl().bind("func"));
	case 4:
		return dynamic::VariantMatcher::SingleMatcher(
		  cxxDestructorDecl().bind("func"));
	}
}

struct MyMatchCallback : public cam::MatchFinder::MatchCallback {
	MyMatchCallback() : count(0) {}
	void run(const cam::MatchFinder::MatchResult& result) override;
	unsigned count;
};

void MyMatchCallback::run(const cam::MatchFinder::MatchResult& result)
{
	++count;
	clang::ASTContext& astContext = *result.Context;
	clang::SourceManager& sourceManager = astContext.getSourceManager();
	std::unique_ptr<clang::MangleContext> mangleContext(
	  astContext.createMangleContext());

	std::string type;
	std::string name;
	std::string mangledName;
	clang::SourceRange sourceRange;

	if (auto qualTypePtr = result.Nodes.getNodeAs<clang::QualType>("type")) {
		if (!qualTypePtr->isNull() && !(*qualTypePtr)->isDependentType()) {
			type = "type";
			name = qualTypePtr->getAsString();
			mangledName = getMangledName(*mangleContext, *qualTypePtr);
		}
	} else if (auto funcDecl =
	  result.Nodes.getNodeAs<clang::FunctionDecl>("func")) {
		sourceRange = funcDecl->getSourceRange();
		name = funcDecl->getQualifiedNameAsString();
		if (auto ctorDecl =
		  llvm::dyn_cast<clang::CXXConstructorDecl>(funcDecl)) {
			type = "constructor\n";
			// TODO/FIXME: The constructor type should be set correctly here.
			mangledName = getMangledName(*mangleContext,
			  clang::GlobalDecl(ctorDecl, clang::CXXCtorType::Ctor_Complete));
		} else if (auto dtorDecl =
		  llvm::dyn_cast<clang::CXXDestructorDecl>(funcDecl)) {
			type = "destructor";
			// TODO/FIXME: The destructor type should be set correctly here.
			mangledName = getMangledName(*mangleContext,
			  clang::GlobalDecl(dtorDecl, clang::CXXDtorType::Dtor_Complete));
		} else {
			type = "function";
			mangledName = getMangledName(*mangleContext, funcDecl);
		}
	} else if (auto varDecl =
	  result.Nodes.getNodeAs<clang::VarDecl>("var")) {
		type = "variable";
		name = varDecl->getQualifiedNameAsString();
		sourceRange = varDecl->getSourceRange();
		if (!varDecl->isLocalVarDeclOrParm()) {
			mangledName = getMangledName(*mangleContext, varDecl);
		}
	} else {
		return;
	}

	if (!clVerbose && mangledName.empty()) {
		return;
	}
	llvm::outs() << std::format("MATCH {}: {} {} {}\n", count, type,
	  !name.empty() ? name : "(null)",
	  !mangledName.empty() ? mangledName : "(null)");
	auto [sourceText, sourceTextValid] = getSourceText(astContext,
	  sourceRange, nullptr);
	if (sourceTextValid) {
		llvm::outs() << std::format("{}\n{}\n",
		  expLocToString(sourceManager, sourceRange.getBegin()), sourceText);
	}
	if (!mangledName.empty()) {
		std::string s = getDemangledName(mangledName);
		if (s != name) {
			llvm::outs() << std::format("MISMATCH {} != {}\n", s, name);
		}
		llvm::ItaniumPartialDemangler demangler;
		assert(mangledName.c_str());
		if (!demangler.partialDemangle(mangledName.c_str())) {
			std::string funcBaseName =
			  demanglerGetFunctionBaseName(demangler);
			std::string funcDeclContextName =
			  demanglerGetFunctionDeclContextName(demangler);
			std::string funcName = demanglerGetFunctionName(demangler);
			std::string funcParms = demanglerGetFunctionParameters(demangler);
			std::string funcRet = demanglerGetFunctionReturnType(demangler);
			if (!funcName.empty()) {
				llvm::outs() << std::format("function name: {}\n", funcName);
			}
			if (!funcBaseName.empty()) {
				llvm::outs() << std::format("function basename: {}\n",
				  funcBaseName);
			}
			if (!funcRet.empty()) {
				llvm::outs() << std::format("function return type: {}\n",
				  funcRet);
			}
			if (!funcParms.empty()) {
				llvm::outs() << std::format("function parameter types: {}\n",
				  funcParms);
			}
			llvm::outs()
			  << std::format("has function qualifiers: {}\n",
			  demangler.hasFunctionQualifiers())
			  << std::format("is constructor/destructor: {}\n",
			  demangler.isCtorOrDtor())
			  << std::format("is function: {}\n",
			  demangler.isFunction())
			  << std::format("is data: {}\n",
			  demangler.isData())
			  << std::format("is special name: {}\n",
			  demangler.isSpecialName())
			  << '\n'
			  ;
		}
	}
}

int main(int argc, const char **argv)
{
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
	std::vector<int> matcherIds(!clMatcherIds.empty() ? clMatcherIds :
	  defaultMatcherIds);
	for (auto id : matcherIds) {
		matchFinder.addDynamicMatcher(*getMatcher(id).getSingleMatcher(),
		  &matchCallback);
	}
	int status = tool.run(ct::newFrontendActionFactory(&matchFinder).get());
	llvm::outs() << std::format("number of matches: {}\n",
	  matchCallback.count);
	return !status ? 0 : 1;
}
