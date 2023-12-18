#include <format>
#include <utility>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include "utility.hpp"

namespace ct = clang::tooling;

int main(int argc, char** argv) {
	std::string errString;
	if (argc < 2) {
		llvm::errs() << "no JSON compilation database specified\n";
		return 1;
	}
	std::string pathname = argv[1];
	std::unique_ptr<ct::CompilationDatabase> compDatabase;
	compDatabase = ct::JSONCompilationDatabase::loadFromFile(pathname,
	  errString, ct::JSONCommandLineSyntax::AutoDetect);
	if (!compDatabase) {
		llvm::errs() << std::format("ERROR: {}\n", errString);
		return 1;
	}
	std::vector<std::string> sourcePaths = compDatabase->getAllFiles();
	for (const auto& sourcePath : sourcePaths) {
		llvm::outs() << std::format("{}\n", sourcePath);
	}
	std::vector<ct::CompileCommand> compCommands =
	  compDatabase->getAllCompileCommands();
	printCompCommands(llvm::outs(), compCommands);
	for (int i = 2; i < argc; ++i) {
		std::vector<ct::CompileCommand> compCommands =
		  compDatabase->getCompileCommands(argv[i]);
		printCompCommands(llvm::outs(), compCommands);
	}
	return 0;
}
