#include <format>
#include <string>
#include <utility>
#include <vector>
#include <unistd.h>
#include <clang/Tooling/ArgumentsAdjusters.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include "utility.hpp"

namespace ct = clang::tooling;

std::pair<std::unique_ptr<ct::CompilationDatabase>, std::string>
  loadCompDatabase(const std::string& pathname, bool json) {
	std::unique_ptr<ct::CompilationDatabase> compDatabase;
	std::string errString;
	if (json) {
		compDatabase = ct::JSONCompilationDatabase::loadFromFile(pathname,
		  errString, ct::JSONCommandLineSyntax::AutoDetect);
	} else {
		compDatabase = ct::FixedCompilationDatabase::loadFromFile(pathname,
		  errString);
	}
	if (compDatabase) {errString.clear();}
	return {std::move(compDatabase), errString};
}

std::unique_ptr<ct::CompilationDatabase> wrapCompDatabase(
  std::unique_ptr<ct::CompilationDatabase> compDatabase, int adjust) {
	compDatabase = std::make_unique<ct::ArgumentsAdjustingCompilations>(
	  std::move(compDatabase));
	auto aac = static_cast<ct::ArgumentsAdjustingCompilations*>(
	  compDatabase.get());
	switch (adjust) {
	case 1:
		aac->appendArgumentsAdjuster(ct::getClangSyntaxOnlyAdjuster());
		break;
	case 2:
		aac->appendArgumentsAdjuster(ct::getInsertArgumentAdjuster("-DFOO",
		  ct::ArgumentInsertPosition::BEGIN));
		break;
	}
	return compDatabase;
}

void processCommandLine(int argc, char** argv, std::string& pathname,
  bool& json, int& adjust, std::vector<std::string>& sourcePathnames) {
	json = true;
	pathname.clear();
	adjust = 0;
	sourcePathnames.clear();
	for (int c; (c = getopt(argc, argv, "a:j:f:")) >= 0;) {
		switch (c) {
		case 'a':
			adjust = std::atoi(optarg);
			break;
		case 'j':
			pathname = optarg;
			json = true;
			break;
		case 'f':
			pathname = optarg;
			json = false;
			break;
		}
	}
	if (pathname.empty()) {
		llvm::errs() << "ERROR: no compilation database specified\n";
		std::exit(1);
	}
	for (int i = optind; i < argc; ++i) {sourcePathnames.push_back(argv[i]);}
}

int main(int argc, char** argv) {
	std::string pathname;
	bool json;
	int adjust;
	std::vector<std::string> sourcePathnames;
	processCommandLine(argc, argv, pathname, json, adjust, sourcePathnames);
	auto [compDatabase, errString] = loadCompDatabase(pathname, json);
	if (!compDatabase) {
		llvm::errs() << std::format("ERROR: {}\n", errString);
		return 1;
	}
	compDatabase = wrapCompDatabase(std::move(compDatabase), adjust);
	std::vector<std::string> sourcePaths = compDatabase->getAllFiles();
	for (const auto& sourcePath : sourcePaths) {
		llvm::outs() << std::format("{}\n", sourcePath);
	}
	std::vector<ct::CompileCommand> compCommands =
	  compDatabase->getAllCompileCommands();
	printCompCommands(llvm::outs(), compCommands);
	for (auto sourcePathname : sourcePathnames) {
		std::vector<ct::CompileCommand> compCommands =
		  compDatabase->getCompileCommands(sourcePathname);
		printCompCommands(llvm::outs(), compCommands);
	}
	return 0;
}
