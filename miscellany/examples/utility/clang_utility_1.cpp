#include <memory>
#include <string>

#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>

#include "clang_utility_1.hpp"

std::string getSourceText(const clang::SourceManager &sourceManager,
  const clang::LangOptions& langOpts, clang::SourceRange sourceRange) {

	assert(sourceRange.isValid());
	clang::SourceLocation beginLoc = sourceRange.getBegin();
	clang::SourceLocation endLoc = sourceRange.getEnd();
	if (beginLoc.isMacroID() || endLoc.isMacroID()) {
		return "@@SOURCE::MACRO@@";
	}

	assert(beginLoc == sourceManager.getSpellingLoc(beginLoc));
	assert(endLoc == sourceManager.getSpellingLoc(endLoc));
	endLoc = clang::Lexer::getLocForEndOfToken(endLoc, 0, sourceManager,
	  langOpts);
	assert(beginLoc.isValid() && endLoc.isValid());
	clang::FileID beginFileId = sourceManager.getFileID(beginLoc);
	clang::FileID endFileId = sourceManager.getFileID(endLoc);
	if (beginFileId != endFileId) {
		return "@@SOURCE::SPANS_FILES@@";
	}

	assert(beginLoc.isValid() && endLoc.isValid());
	unsigned beginOffset = sourceManager.getFileOffset(beginLoc);
	unsigned endOffset = sourceManager.getFileOffset(endLoc);
	std::size_t length = endOffset - beginOffset;
	llvm::StringRef buffer = sourceManager.getBufferData(beginFileId);
	return std::string(buffer.substr(beginOffset, length));
}
