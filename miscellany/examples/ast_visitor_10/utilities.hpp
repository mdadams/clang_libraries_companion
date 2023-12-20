#include <string>
#include <clang/AST/Decl.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/SourceLocation.h>

std::string locationToString(const clang::SourceManager& sourceManager,
  clang::SourceLocation sourceLoc, bool includeFileName = true);

std::string rangeToString(const clang::SourceManager& sourceManager,
  clang::SourceRange sourceRange, bool includeFileName = true);

std::string getSourceTextRaw(const clang::SourceManager& sm,
  clang::SourceRange range);

std::string getSourceText(const clang::SourceManager& sm,
  clang::SourceRange range);

std::string addLineNumbers(const std::string& source, unsigned int startLine,
  unsigned int startCol = 1, bool includeHeader = true);

std::string getSourceTextWithLineNumbers(clang::SourceManager& sourceManager,
  clang::SourceRange sourceRange, bool includeHeader = true);

std::string functionDeclTemplatedKindToString(clang::FunctionDecl::TemplatedKind kind);
