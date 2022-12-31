#include <format>
#include <tuple>
#include <string>
#include "clang/Lex/Lexer.h"
#include "utilities2.hpp"

clang::SourceLocation getEndOfToken(const clang::SourceManager& sourceManager,
  clang::SourceLocation startOfToken) {
	return clang::Lexer::getLocForEndOfToken(startOfToken, 0, sourceManager,
	  clang::LangOptions());
}

std::tuple<std::string, unsigned, unsigned> getFlc(const clang::SourceManager&
  sourceManager, clang::SourceLocation sourceLocation, bool spelling) {
	if (spelling) {
		return std::tuple<std::string, unsigned, unsigned>{
		  sourceManager.getFilename(sourceManager.getSpellingLoc(
		  sourceLocation)), sourceManager.getSpellingLineNumber(sourceLocation),
		  sourceManager.getSpellingColumnNumber(sourceLocation)};
	} else {
		return std::tuple<std::string, unsigned, unsigned>{
		  sourceManager.getFilename(sourceManager.getExpansionLoc(
		  sourceLocation)), sourceManager.getExpansionLineNumber(
		  sourceLocation), sourceManager.getExpansionColumnNumber(
		  sourceLocation)};
	}
}

std::string locationToString(const clang::SourceManager& sourceManager,
  clang::SourceLocation sourceLocation, bool spelling) {
	auto [filename, lineNum, columnNum] = getFlc(sourceManager,
	  sourceLocation, spelling);
	return std::format("{}:{}({})", filename, lineNum, columnNum);
}

std::string rangeToString(const clang::SourceManager& sourceManager,
  clang::SourceRange sourceRange, bool spelling) {
	auto [beginFilename, beginLineNum, beginColumnNum] = getFlc(sourceManager,
	  sourceRange.getBegin(), spelling);
	auto [endFilename, endLineNum, endColumnNum] = getFlc(sourceManager,
	  sourceRange.getEnd(), spelling);
	return std::format("{}:{}({})-{}({})", beginFilename, beginLineNum,
	  beginColumnNum, endLineNum, endColumnNum);
}

std::string getSourceTextRaw(const clang::SourceManager& sourceManager,
  clang::SourceRange sourceRange) {
	assert(sourceRange.isValid());
	return std::string(clang::Lexer::getSourceText(
	  clang::CharSourceRange::getCharRange(sourceRange), sourceManager,
	  clang::LangOptions()));
}

std::string getSourceText(const clang::SourceManager& sourceManager,
  clang::SourceRange sourceRange) {
	assert(sourceRange.isValid());
	//sourceRange.setEnd(getEndOfToken(sourceManager, sourceRange.getEnd()));
	auto beginFileId = sourceManager.getFileID(sourceRange.getBegin());
	auto endFileId = sourceManager.getFileID(sourceRange.getEnd());
	bool invalid = false;
	bool check = (sourceManager.isWrittenInSameFile(sourceRange.getBegin(),
	  sourceRange.getEnd()));
	if (!check) {
		return "[invalid !isWrittenInSameFile]";
	}
	std::string result = std::string(clang::Lexer::getSourceText(
	  clang::CharSourceRange::getCharRange(sourceRange), sourceManager,
	  clang::LangOptions(), &invalid));
	//assert(!invalid || (invalid && !check));
	return !invalid ? result :
	  std::format("[invalid getSourceText failed {} {} {} {}]", check,
	  beginFileId.isValid(), endFileId.isValid(), beginFileId == endFileId);
}

std::string addLineNumbers(const std::string& source, unsigned int startLine,
  unsigned int startCol, bool includeHeader) {
	std::string result;
	bool needPrefix = true;
	for (auto c : source) {
		if (needPrefix) {
			if (includeHeader) {
				result += "      ";
				result += "0000000001111111111222222222233333333334444444444";
				result += "555555555566666666667";
				result += "\n";
				result += "      ";
				result += "12345678901234567890123456789012345678901234567890";
				result += "12345678901234567890";
				result += "\n";
				includeHeader = false;
			}
			result += std::format("{:4d}: {}", startLine, std::string(
			  startCol - 1, ' '));
			needPrefix = false;
		}
		result += c;
		++startCol;
		if (c == '\n') {
			++startLine;
			startCol = 1;
			needPrefix = true;
		}
	}
	if (startCol > 1) {result += '\n';}
	return result;
}

std::string getSourceTextWithLineNumbers(const clang::SourceManager&
  sourceManager, clang::SourceRange sourceRange, bool spelling, bool header) {
	auto [dummy, lineNum, columnNum] = getFlc(sourceManager,
	  sourceRange.getBegin(), spelling);
	return addLineNumbers(getSourceText(sourceManager, sourceRange),
	  lineNum, columnNum, header);
}
