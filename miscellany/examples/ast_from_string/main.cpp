#include <format>
#include <memory>
#include <string>
#include <vector>
#include <clang/Basic/Diagnostic.h>
#include "clang/Frontend/ASTUnit.h"
#include "clang/Serialization/PCHContainerOperations.h"
#include "clang/Tooling/ArgumentsAdjusters.h"
#include "clang/Tooling/Tooling.h"

namespace ct = clang::tooling;

int processSourceText(const std::string& text) {
	auto diagConsumer = std::make_unique<clang::DiagnosticConsumer>();
	std::unique_ptr<clang::ASTUnit> ast(ct::buildASTFromCodeWithArgs(
	  text, std::vector<std::string>(), "input.cpp", "clang-tool",
	  std::make_shared<clang::PCHContainerOperations>(),
	  ct::getClangStripDependencyFileAdjuster(),
	  ct::FileContentMappings(), diagConsumer.get()));
	auto numErrs = diagConsumer->getNumErrors();
	auto numWarns = diagConsumer->getNumWarnings();
	if (numErrs) {
		llvm::outs() << std::format("number of errors: {}\n", numErrs);
	}
	if (numWarns) {
		llvm::outs() << std::format("number of warnings: {}\n", numWarns);
	}
	if (numErrs) {
		llvm::outs() << std::format("processing failed\n");
		return numErrs;
	}
	clang::TranslationUnitDecl* tuDecl =
	  ast->getASTContext().getTranslationUnitDecl();
	if (tuDecl) {
		llvm::outs() << std::format("{:=^40s}\n", " start of AST dump ");
		tuDecl->dump();
		llvm::outs() << std::format("{:=^40s}\n", " end of AST dump ");
	}
	return 0;
}

int main(int argc, char** argv) {
	int failCount = 0;
	for (int i = 1; i < argc; ++i) {
		llvm::outs() << std::format("PROCESSING INPUT {}\n", i);
		if (processSourceText(argv[i])) {
			++failCount;
		}
	}
	if (failCount) {
		llvm::outs() << std::format("number of inputs with failures: {}\n",
		  failCount);
	}
	return !failCount ? 0 : 1;
}
