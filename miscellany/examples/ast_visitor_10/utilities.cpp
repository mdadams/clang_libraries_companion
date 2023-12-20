#include <format>
#include <string>

#include <clang/Basic/SourceManager.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Lex/Lexer.h>

#include "utilities.hpp"

std::string locationToString(const clang::SourceManager& sourceManager,
  clang::SourceLocation sourceLoc, bool includeFileName) {
	assert(sourceLoc.isValid());
	std::string s = includeFileName ? std::string(
	  sourceManager.getFilename(sourceLoc)) + ":" : "";
	return s + std::format("{}({})",
	  sourceManager.getSpellingLineNumber(sourceLoc),
	  sourceManager.getSpellingColumnNumber(sourceLoc));
}

std::string rangeToString(const clang::SourceManager& sourceManager,
  clang::SourceRange sourceRange, bool includeFileName) {
	assert(sourceRange.isValid());
	std::string result;
	//assert(!sourceManager.getFilename(sourceRange.getBegin()).empty());
	//assert(!sourceManager.getFilename(sourceRange.getEnd()).empty());
	if (includeFileName) {
		std::string filename{sourceManager.getFilename(
		  sourceRange.getBegin())};
		result += !filename.empty() ? filename : "[unknown]";
		result += ':';
	}
	result += locationToString(sourceManager, sourceRange.getBegin(), false);
	result += '-';
	if (includeFileName) {
		std::string filename{sourceManager.getFilename(sourceRange.getEnd())};
		result += !filename.empty() ? filename : "[unknown]";
		result += ':';
	}
	result += locationToString(sourceManager, sourceRange.getEnd(), false);
	return result;
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
	clang::LangOptions lo;
	auto startLoc = sourceManager.getSpellingLoc(sourceRange.getBegin());
	auto lastTokenLoc = sourceManager.getSpellingLoc(sourceRange.getEnd());
	auto endLoc = clang::Lexer::getLocForEndOfToken(lastTokenLoc, 0,
	  sourceManager, lo);
	auto printableRange = clang::SourceRange{startLoc, endLoc};
	return getSourceTextRaw(sourceManager, printableRange);
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

std::string getSourceTextWithLineNumbers(clang::SourceManager& sourceManager,
  clang::SourceRange sourceRange, bool includeHeader) {
	assert(sourceRange.isValid());
	std::string s = getSourceText(sourceManager, sourceRange);
	return addLineNumbers(s,
	  sourceManager.getSpellingLineNumber(sourceRange.getBegin()),
	  sourceManager.getSpellingColumnNumber(sourceRange.getBegin()),
	  includeHeader);
}

std::string functionDeclTemplatedKindToString(
  clang::FunctionDecl::TemplatedKind kind) {
	std::map<clang::FunctionDecl::TemplatedKind, std::string> lut{
		{clang::FunctionDecl::TemplatedKind::TK_NonTemplate,
		  "nontemplate"},
		{clang::FunctionDecl::TemplatedKind::TK_FunctionTemplate,
		  "function template"},
		{clang::FunctionDecl::TemplatedKind::TK_MemberSpecialization,
		  "member specialization"},
		{clang::FunctionDecl::TemplatedKind::TK_FunctionTemplateSpecialization,
		  "function template specialization"},
		{clang::FunctionDecl::TemplatedKind::TK_DependentFunctionTemplateSpecialization,
		  "dependent function template specialization"}
	};
	auto i = lut.find(kind);
	assert(i != lut.end());
	return i != lut.end() ? i->second : "";
}
