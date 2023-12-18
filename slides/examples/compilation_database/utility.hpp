#include <vector>
#include <llvm/Support/raw_ostream.h>

bool printCompCommands(llvm::raw_fd_ostream& out,
  const std::vector<clang::tooling::CompileCommand>& compCommands);
