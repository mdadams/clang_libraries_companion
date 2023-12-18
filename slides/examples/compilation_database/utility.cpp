#include <format>
#include <vector>
#include <clang/Tooling/CompilationDatabase.h>
#include <llvm/Support/raw_ostream.h>
#include "utility.hpp"

namespace ct = clang::tooling;

bool printCompCommands(llvm::raw_fd_ostream& out,
  const std::vector<ct::CompileCommand>& compCommands) {
	for (auto compCommand = compCommands.begin();
	  compCommand != compCommands.end(); ++compCommand) {
		out << "command:\n"
		  << std::format("  filename: {}\n", compCommand->Filename)
		  << std::format("  directory: {}\n", compCommand->Directory);
		out << "  command line:";
		for (auto word : compCommand->CommandLine) {out << " " << word;}
		out << '\n';
		if (!compCommand->Output.empty()) {
			out << "  output: " << compCommand->Output << '\n';
		} else {
			out << "  no output\n";
		}
		if (!compCommand->Heuristic.empty()) {
			out << "  heuristic: " << compCommand->Heuristic << '\n';
		} else {
			out << "  no heuristic\n";
		}
	}
	return !out.has_error();
}
