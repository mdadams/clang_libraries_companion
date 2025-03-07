#include <format>
#include <memory>
#include <string>
#include <vector>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>

namespace lc = llvm::cl;
namespace ct = clang::tooling;

static lc::opt<std::string> sourcePath(lc::Positional,
  lc::desc("<source file>"), lc::Required);
static lc::opt<std::string> astPath("o", lc::desc("Output AST file"),
  lc::Required);
static lc::opt<std::string> clangIncDir("clang-include-dir",
  lc::desc("Output AST file"), lc::Required);
static lc::list<std::string> extraArgs("extra-arg",
  lc::desc("extra arg"), lc::ZeroOrMore);

std::unique_ptr<llvm::MemoryBuffer> loadFile(const std::string& path) {
	llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> bufferOrError{
	  llvm::MemoryBuffer::getFile(path)};
	if (!bufferOrError) {return nullptr;}
	return std::move(*bufferOrError);
}

int main(int argc, const char** argv) {
	lc::ParseCommandLineOptions(argc, argv);
	std::unique_ptr<llvm::MemoryBuffer> sourceCode = loadFile(sourcePath);
	if (!sourceCode) {
		llvm::errs() << "cannot load source file\n";
		return 1;
	}
	std::vector<std::string> args{
		std::format("-I{}", std::string(clangIncDir)),
	};
	for (auto arg : extraArgs) {args.push_back(arg);}
	std::unique_ptr<clang::ASTUnit> astUnit = ct::buildASTFromCodeWithArgs(
	  sourceCode->getBuffer(), args, sourcePath, "save_ast");
	if (!astUnit) {
		llvm::errs() << "failed to build AST\n";
		return 1;
	}
	if (astUnit->Save(astPath)) {
		llvm::errs() << "cannot save AST file\n";
		return 1;
	}
	return 0;
}
