#include <string>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/SourceLocation.h>

std::string locationToString(const clang::SourceManager& sourceManager,
  clang::SourceLocation sourceLoc);

std::string rangeToString(const clang::SourceManager& sourceManager,
  clang::SourceRange sourceRange);

std::string getSourceTextRaw(const clang::SourceManager& sm,
  clang::SourceRange range);

std::string getSourceText(const clang::SourceManager& sm,
  clang::SourceRange range);

std::string addLineNumbers(const std::string& source, unsigned int start);
