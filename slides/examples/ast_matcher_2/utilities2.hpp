#include <string>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/SourceLocation.h>

std::string getSourceTextRaw(const clang::SourceManager& sourceManager,
  clang::SourceRange sourceRange);
clang::SourceLocation getEndOfToken(const clang::SourceManager& sourceManager,
  clang::SourceLocation startOfToken);
std::string locationToString(const clang::SourceManager& sourceManager,
  clang::SourceLocation sourceLoc, bool spelling);
std::string rangeToString(const clang::SourceManager& sourceManager,
  clang::SourceRange sourceRange, bool spelling);
std::string getSourceText(const clang::SourceManager& sourceManager,
  clang::SourceRange range);
std::string getSourceTextWithLineNumbers(const clang::SourceManager&
  sourceManager, clang::SourceRange sourceRange, bool spelling, bool header);
std::string addLineNumbers(const std::string& source, unsigned int startLineNo,
  unsigned int startColNo, bool header);
