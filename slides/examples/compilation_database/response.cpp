#include <cstdlib>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include "utility.hpp"

namespace ct = clang::tooling;

std::unique_ptr<ct::CompilationDatabase> loadCompDatabase(
  std::string_view path, bool expand, std::string& errString) {
	errString.clear();
	std::unique_ptr<ct::CompilationDatabase> compDatabase{
	  ct::JSONCompilationDatabase::loadFromFile(path, errString,
	  ct::JSONCommandLineSyntax::AutoDetect)};
	if (!compDatabase) {return nullptr;}
	if (expand) {
		compDatabase = ct::expandResponseFiles(std::move(compDatabase),
		  llvm::vfs::getRealFileSystem());
		if (!compDatabase) {return nullptr;}
	}
	return compDatabase;
}

int main(int argc, char** argv) {
	if (argc < 3) {
		llvm::errs() << "no compilation database or expand flag specified\n";
		return 1;
	}
	std::string path = argv[1];
	bool expand = std::atoi(argv[2]);
	std::string errString;
	std::unique_ptr<ct::CompilationDatabase> compDatabase{
	  loadCompDatabase(path, expand, errString)};
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
	if (argc <= 3) {printCompCommands(llvm::outs(), compCommands);}
	for (int i = 3; i < argc; ++i) {
		std::vector<ct::CompileCommand> compCommands =
		  compDatabase->getCompileCommands(argv[i]);
		printCompCommands(llvm::outs(), compCommands);
	}
	return 0;
}
