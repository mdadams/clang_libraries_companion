#include <string>
#include <clang/ASTMatchers/ASTMatchers.h>

namespace cam = clang::ast_matchers;

cam::DeclarationMatcher matchFuncDef() {
	using namespace cam;
	return functionDecl(isDefinition()).bind("func");
}

cam::DeclarationMatcher matchFuncDeclOf(const std::string& funcName) {
	using namespace cam;
	return functionDecl(hasName(funcName)).bind("func");
}

cam::StatementMatcher matchCallTo(const std::string& funcName) {
	using namespace cam;
	return callExpr(callee(
	  functionDecl(hasName(funcName)).bind("func"))).bind("call");
}

cam::TypeMatcher matchPointerType() {
	using namespace cam;
	return qualType(isAnyPointer());
}
