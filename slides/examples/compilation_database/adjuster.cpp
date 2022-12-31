#include <format>
#include <utility>
#include <unistd.h>
#include "clang/Tooling/ArgumentsAdjusters.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Signals.h"
#include "utility.hpp"

namespace ct = clang::tooling;

int main(int argc, char** argv) {
	int json = -1;
	std::string pathname;
	int adjust = 0;
	for (int c; (c = getopt(argc, argv, "a:j:f:")) >= 0;) {
		switch (c) {
		case 'a':
			adjust = std::atoi(optarg);
			break;
		case 'j':
			pathname = optarg;
			json = 1;
			break;
		case 'f':
			pathname = optarg;
			json = 0;
			break;
		}
	}
	if (json != 0 && json != 1) {
		llvm::errs() << "ERROR: no compilation database specified\n";
		return 1;
	}
	std::string errString;
	std::unique_ptr<ct::CompilationDatabase> compDatabase;
	switch (json) {
	case 0:
		compDatabase = ct::FixedCompilationDatabase::loadFromFile(pathname,
		  errString);
		break;
	case 1:
		compDatabase = ct::JSONCompilationDatabase::loadFromFile(pathname,
		  errString, ct::JSONCommandLineSyntax::AutoDetect);
		break;
	}
	if (!compDatabase) {
		llvm::errs() << std::format("ERROR: {}\n", errString);
		return 1;
	}
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
	std::vector<std::string> sourcePaths = compDatabase->getAllFiles();
	for (const auto& sourcePath : sourcePaths) {
		llvm::outs() << std::format("{}\n", sourcePath);
	}
	std::vector<ct::CompileCommand> compCommands =
	  compDatabase->getAllCompileCommands();
	printCompCommands(llvm::outs(), compCommands);
	for (; optind < argc; ++optind) {
		std::vector<ct::CompileCommand> compCommands =
		  compDatabase->getCompileCommands(argv[optind]);
		printCompCommands(llvm::outs(), compCommands);
	}
	return 0;
}
