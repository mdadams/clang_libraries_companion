#include <format>
#include <memory>
#include <string>
#include <vector>
#include <clang/AST/DeclVisitor.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>

namespace lc = llvm::cl;
namespace ct = clang::tooling;

static lc::opt<std::string> sourcePath(lc::Positional, lc::Required);
static lc::opt<std::string> clangIncDir("clang-include-dir", lc::Required);
static lc::list<std::string> extraArgs("extra-arg", lc::ZeroOrMore);

class MyDeclVisitor : public clang::ConstDeclVisitor<MyDeclVisitor> {
public:
	void VisitFunctionDecl(const clang::FunctionDecl* funcDecl) {
		llvm::outs() << std::format("FunctionDecl {}\n",
		  funcDecl->getNameAsString());
	}
	void VisitNamedDecl(const clang::NamedDecl* namedDecl) {
		llvm::outs() << std::format("NamedDecl {}\n",
		  namedDecl->getNameAsString());
	}
	void VisitDecl(const clang::Decl* decl) {
		llvm::outs() << std::format("not handled {} ({})\n",
		  decl->getDeclKindName(), decl->isImplicit() ? "implicit" :
		  "not implicit");
	}
};

void processAst(clang::ASTContext& astContext) {
	clang::TranslationUnitDecl* tuDecl{astContext.getTranslationUnitDecl()};
	auto declContext = llvm::dyn_cast<clang::DeclContext>(tuDecl);
	for (auto i = declContext->decls_begin(); i != declContext->decls_end();
	  ++i) {
		const clang::Decl* decl = *i;
		MyDeclVisitor visitor;
		visitor.Visit(decl);
	}
}

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
	  sourceCode->getBuffer(), args, sourcePath);
	if (!astUnit) {
		llvm::errs() << "failed to build AST\n";
		return 1;
	}
	processAst(astUnit->getASTContext());
	return 0;
}
