#include <string>
#include "clang/Frontend/ASTUnit.h"
#include "clang/Tooling/Tooling.h"

namespace ct = clang::tooling;

void process(const std::string& text) {
	std::unique_ptr<clang::ASTUnit> ast(ct::buildASTFromCode(text));
	clang::TranslationUnitDecl* tuDecl =
	  ast->getASTContext().getTranslationUnitDecl();
	if (tuDecl) {
		llvm::errs() << "---------dump begin----------\n";
		tuDecl->dump();
		llvm::errs() << "---------dump end----------\n";
	}
}

int main(int argc, char** argv) {
	for (int i = 1; i < argc; ++i) {
		process(argv[i]);
	}
}
