#include <format>
#include <tuple>
#include <boost/process/search_path.hpp>
#include <boost/process.hpp>
#include "clang/Lex/Lexer.h"
#include "clang_utility.hpp"

namespace bf = boost::filesystem;
namespace bp = boost::process;

std::pair<bool, std::string> sourceRangeToText(const clang::SourceManager&
  sourceManager, clang::SourceRange sourceRange, bool tokenRange) {
	if (sourceRange.isValid()) {
		bool invalid = true;
		auto text = std::string(clang::Lexer::getSourceText(
		  (tokenRange ? clang::CharSourceRange::getTokenRange(sourceRange) :
		  clang::CharSourceRange::getCharRange(sourceRange)),
		  sourceManager,
		  clang::LangOptions(), &invalid));
		return {!invalid, text};
	} else {
		return {false, ""};
	}
}

std::pair<bool, std::string> charSourceRangeToText(const clang::SourceManager&
  sourceManager, clang::CharSourceRange charSourceRange) {
	if (charSourceRange.isValid()) {
		bool invalid = true;
		auto text = std::string(clang::Lexer::getSourceText(charSourceRange,
		  sourceManager, clang::LangOptions(), &invalid));
		return {!invalid, text};
	} else {
		return {false, ""};
	}
}

clang::SourceLocation getBeginningOfToken(const clang::SourceManager&
  sourceManager, clang::SourceLocation loc) {
	return clang::Lexer::GetBeginningOfToken(loc, sourceManager,
	  clang::LangOptions());
}

clang::SourceLocation getEndOfToken(const clang::SourceManager& sourceManager,
  clang::SourceLocation startOfToken) {
	return clang::Lexer::getLocForEndOfToken(startOfToken, 0, sourceManager,
	  clang::LangOptions());
}
