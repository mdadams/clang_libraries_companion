#include <format>
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Lexer.h"

std::string locationToString(const clang::SourceManager& sourceManager,
  clang::SourceLocation sourceLoc) {
	return std::format("{}:{}({})",
	  std::string(sourceManager.getFilename(sourceLoc)),
	  sourceManager.getSpellingLineNumber(sourceLoc),
	  sourceManager.getSpellingColumnNumber(sourceLoc));
}

std::string rangeToString(const clang::SourceManager& sourceManager,
  clang::SourceRange sourceRange) {
	std::string beginFilename(sourceManager.getFilename(
	  sourceRange.getBegin()));
	std::string endFilename(sourceManager.getFilename(sourceRange.getEnd()));
	return std::format("{}:{}({})-{}{}({})", beginFilename,
	  sourceManager.getSpellingLineNumber(sourceRange.getBegin()),
	  sourceManager.getSpellingColumnNumber(sourceRange.getBegin()),
	  endFilename != beginFilename ? endFilename + ":" : "",
	  sourceManager.getSpellingLineNumber(sourceRange.getEnd()),
	  sourceManager.getSpellingColumnNumber(sourceRange.getEnd()));
}

std::string getSourceText(const clang::SourceManager& sourceManager,
  clang::SourceRange range) {
	return std::string(clang::Lexer::getSourceText(
	  clang::CharSourceRange::getTokenRange(range), sourceManager,
	  clang::LangOptions()));
}

std::string addLineNumbers(const std::string& source, unsigned int start) {
	std::string result;
	result += std::format("{:4d}: ", start);
	for (auto c : source) {
		if (c == '\n') {
			++start;
			result += std::format("\n{:4d}: ", start);
		} else {result += c;}
	}
	return result;
}
