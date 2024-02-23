/****************************************************************************\
* Includes
\****************************************************************************/

#include <format>
#include <string>

#include <clang/AST/Mangle.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/Dynamic/VariantValue.h>
#include <clang/AST/Type.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Demangle/Demangle.h>
#include <llvm/Support/CommandLine.h>

/****************************************************************************\
\****************************************************************************/

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;
namespace lc = llvm::cl;

enum class MatcherId {
	Type,
	Var,
	Func,
	Ctor,
	Dtor,
};

std::string matcherIdToName(MatcherId id) {
	std::map<MatcherId, std::string> lut{
		{MatcherId::Type, "type"},
		{MatcherId::Var, "var"},
		{MatcherId::Func, "func"},
		{MatcherId::Ctor, "ctor"},
		{MatcherId::Dtor, "dtor"},
	};
	auto i = lut.find(id);
	return i != lut.end() ? i->second : "";
}

/****************************************************************************\
* Command-Line Processing
\****************************************************************************/

static const std::vector<MatcherId> defaultMatcherIds{
	MatcherId::Type,
	MatcherId::Var,
	MatcherId::Func,
};

static int clVerbosityLevel = 0;

static lc::OptionCategory optionCategory("Tool options");

static lc::list<MatcherId> clMatcherIds(
  "m",
  lc::desc("Enable matcher"),
  lc::values(
    clEnumValN(MatcherId::Type, "type", "type"),
    clEnumValN(MatcherId::Func, "func", "function"),
    clEnumValN(MatcherId::Var, "var", "variable"),
    clEnumValN(MatcherId::Ctor, "ctor", "constructor"),
    clEnumValN(MatcherId::Dtor, "dtor", "destructor")
  ),
  lc::cat(optionCategory),
  lc::ZeroOrMore
);

static lc::opt<bool> clVerbose(
  "v",
  lc::desc("Increase verbosity level"),
  lc::cat(optionCategory),
  lc::ZeroOrMore,
  lc::callback([](const bool&){
	  ++clVerbosityLevel;
  })
);

/****************************************************************************\
* Source-Manager Related Code.
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
* Name Mangling
\****************************************************************************/

std::string getMangledName(clang::MangleContext& mangleContext,
  clang::QualType qualType)
{
	assert(!qualType.isNull() && !qualType->isDependentType());
	std::string mangledName;
	llvm::raw_string_ostream mangledOut(mangledName);
#if (LLVM_MAJOR_VERSION >= 18)
	mangleContext.mangleCXXRTTI(qualType, mangledOut);
#else
	mangleContext.mangleTypeName(qualType, mangledOut);
#endif
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
* Name Demangling
\****************************************************************************/

std::string getDemangledName(const std::string& mangledName)
{
	std::size_t size = 0;
	int status;
#if (LLVM_MAJOR_VERSION >= 17)
	char* result = llvm::itaniumDemangle(mangledName);
#else /* LLVM 15 and 16 */
	char* result = llvm::itaniumDemangle(mangledName.c_str(), nullptr, &size,
	  &status);
#endif
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
* Matching Infrastructure
\****************************************************************************/

cam::dynamic::VariantMatcher getMatcher(MatcherId id)
{
	using namespace cam;
	switch (id) {
	default:
	case MatcherId::Type:
		return dynamic::VariantMatcher::SingleMatcher(
		  qualType().bind("type"));
	case MatcherId::Var:
		return dynamic::VariantMatcher::SingleMatcher(varDecl().bind("var"));
	case MatcherId::Func:
		return dynamic::VariantMatcher::SingleMatcher(
		  functionDecl().bind("func"));
	case MatcherId::Ctor:
		return dynamic::VariantMatcher::SingleMatcher(
		  cxxConstructorDecl().bind("func"));
	case MatcherId::Dtor:
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
	bool shouldMangle = true;
	std::string dumpOutput;
	llvm::raw_string_ostream dumpStream(dumpOutput);

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
		shouldMangle = mangleContext->shouldMangleDeclName(funcDecl);
		funcDecl->dump(dumpStream);
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
		varDecl->dump(dumpStream);
		sourceRange = varDecl->getSourceRange();
		if (!varDecl->isLocalVarDeclOrParm()) {
			shouldMangle = mangleContext->shouldMangleDeclName(varDecl);
			mangledName = getMangledName(*mangleContext, varDecl);
		} else {
			shouldMangle = false;
		}
	} else {
		return;
	}

	if (!(clVerbosityLevel >= 1) && mangledName.empty()) {
		return;
	}
	llvm::outs() << std::format("MATCH {}: {} {} {} {}\n", count, type,
	  !name.empty() ? name : "(null)", shouldMangle,
	  !mangledName.empty() ? mangledName : "(null)");
	auto [sourceText, sourceTextValid] = getSourceText(astContext,
	  sourceRange, nullptr);
	if (clVerbosityLevel >= 2) {
		llvm::outs() << dumpOutput;
	}
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

/****************************************************************************\
* Main
\****************************************************************************/

int main(int argc, const char **argv)
{
	auto optParser = ct::CommonOptionsParser::create(argc, argv,
	  optionCategory);
	if (!optParser) {
		llvm::errs() << llvm::toString(optParser.takeError());
		return 1;
	}
	if (clVerbosityLevel >= 1) {
		llvm::outs() << std::format("verbosity level: {}\n",
		  clVerbosityLevel);
	}
	ct::ClangTool tool(optParser->getCompilations(),
	  optParser->getSourcePathList());
	MyMatchCallback matchCallback;
	cam::MatchFinder matchFinder;
	std::vector<MatcherId> matcherIds(!clMatcherIds.empty() ? clMatcherIds :
	  defaultMatcherIds);
	for (auto id : matcherIds) {
		if (clVerbosityLevel >= 1) {
			llvm::outs() << std::format("enabling matcher {}\n",
			  matcherIdToName(id));
		}
		matchFinder.addDynamicMatcher(*getMatcher(id).getSingleMatcher(),
		  &matchCallback);
	}
	int status = tool.run(ct::newFrontendActionFactory(&matchFinder).get());
	llvm::outs() << std::format("number of matches: {}\n",
	  matchCallback.count);
	return !status ? 0 : 1;
}
