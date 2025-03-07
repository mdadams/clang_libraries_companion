#ifndef clang_utility_1_hpp
#define clang_utility_1_hpp

#include <string>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>

std::string getSourceText(const clang::SourceManager &sourceManager,
  const clang::LangOptions& langOpts, clang::SourceRange sourceRange);

#endif
