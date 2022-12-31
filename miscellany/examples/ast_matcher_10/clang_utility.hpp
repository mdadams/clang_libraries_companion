#pragma once

#include <string>
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/SourceLocation.h"

std::pair<bool, std::string> charSourceRangeToText(const clang::SourceManager&
  sourceManager, clang::CharSourceRange charSourceRange);
std::pair<bool, std::string> sourceRangeToText(const clang::SourceManager&
  sourceManager, clang::SourceRange sourceRange, bool tokenRange = true);
clang::SourceLocation getBeginningOfToken(const clang::SourceManager&
  sourceManager, clang::SourceLocation loc);
clang::SourceLocation getEndOfToken(const clang::SourceManager& sourceManager,
  clang::SourceLocation startOfToken);
