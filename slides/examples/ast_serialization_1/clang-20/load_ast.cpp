#include <memory>
#include <string>
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/FrontendOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Lex/Lexer.h>
#include <clang/Serialization/PCHContainerOperations.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

namespace lc = llvm::cl;

static lc::opt<std::string> InputASTFile(lc::Positional,
  lc::desc("input AST file"), lc::Required);

std::string getSourceText(const clang::SourceManager &sourceManager,
  const clang::LangOptions& langOpts, clang::SourceRange sourceRange) {
	assert(sourceRange.isValid());
	clang::SourceLocation beginLoc = sourceRange.getBegin();
	clang::SourceLocation endLoc = sourceRange.getEnd();
	if (beginLoc.isMacroID() || endLoc.isMacroID()) {return "@@SOURCE::MACRO@@"; }
	assert(beginLoc == sourceManager.getSpellingLoc(beginLoc));
	assert(endLoc == sourceManager.getSpellingLoc(endLoc));
	endLoc = clang::Lexer::getLocForEndOfToken(endLoc, 0, sourceManager,
	  langOpts);
	assert(beginLoc.isValid() && endLoc.isValid());
	clang::FileID beginFileId = sourceManager.getFileID(beginLoc);
	clang::FileID endFileId = sourceManager.getFileID(endLoc);
	if (beginFileId != endFileId) {return "@@SOURCE::SPANS_FILES@@";}
	assert(beginLoc.isValid() && endLoc.isValid());
	unsigned beginOffset = sourceManager.getFileOffset(beginLoc);
	unsigned endOffset = sourceManager.getFileOffset(endLoc);
	std::size_t length = endOffset - beginOffset;
	llvm::StringRef buffer = sourceManager.getBufferData(beginFileId);
	return std::string(buffer.substr(beginOffset, length));
}

class MyASTVisitor : public clang::RecursiveASTVisitor<MyASTVisitor> {
public:
	explicit MyASTVisitor(clang::ASTUnit &astUnit) :
	  sourceManager_(&astUnit.getSourceManager()),
	  langOpts_(&astUnit.getLangOpts()) {}
	std::string makeHeading(const std::string& title)
	  {return std::format("{:=^80}\n{}\n", "", title);}
	bool VisitDecl(clang::Decl *decl) {
		llvm::outs() << makeHeading(std::format("Decl: {}",
		  std::string(decl->getDeclKindName())));
		clang::SourceRange sourceRange = decl->getSourceRange();
		if (sourceRange.isValid()) {
			llvm::outs() << getSourceText(*sourceManager_, *langOpts_,
			  sourceRange) << '\n';
		}
		return true;
	}
	bool VisitStmt(clang::Stmt *stmt) {
		llvm::outs() << makeHeading(std::format("Stmt: {}",
		  std::string(stmt->getStmtClassName())));
		clang::SourceRange sourceRange = stmt->getSourceRange();
		if (sourceRange.isValid()) {
			llvm::outs() << getSourceText(*sourceManager_, *langOpts_,
			  sourceRange) << '\n';
		}
		return true;
	}
private:
	clang::SourceManager* sourceManager_;
	const clang::LangOptions* langOpts_;
};

int main(int argc, const char** argv) {
	lc::ParseCommandLineOptions(argc, argv, "AST deserializer\n");
	llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diagOpts(
	  new clang::DiagnosticOptions());
	llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diagIDs(
	  new clang::DiagnosticIDs());
	llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine> diagEngine(
	  new clang::DiagnosticsEngine(diagIDs, diagOpts,
	  new clang::TextDiagnosticPrinter(llvm::errs(), &*diagOpts), true));
	clang::PCHContainerOperations pchContainerOps;
	clang::FileSystemOptions fileSysOpts;
	std::unique_ptr<clang::ASTUnit> astUnit = clang::ASTUnit::LoadFromASTFile(
	  InputASTFile, pchContainerOps.getRawReader(),
	  clang::ASTUnit::LoadEverything, diagEngine, fileSysOpts, nullptr);
	if (!astUnit) {
		llvm::errs() << "cannot load AST file\n";
		return 1;
	}
	clang::ASTContext &astContext = astUnit->getASTContext();
	clang::TranslationUnitDecl *tuDecl = astContext.getTranslationUnitDecl();
	llvm::outs() << std::format("main file name: {}\n",
	  std::string_view(astUnit->getMainFileName()));
	llvm::outs() << std::format("language: {}\n",
	  astUnit->getInputKind().getLanguage() == clang::Language::CXX ? "C++" :
	  "other");
	const clang::LangOptions& langOpts = astUnit->getLangOpts();
	llvm::outs() << std::format("language standard unspecified: {}\n",
	  langOpts.LangStd == clang::LangStandard::Kind::lang_unspecified);
	MyASTVisitor visitor(*astUnit);
	visitor.TraverseDecl(tuDecl);
	return 0;
}
