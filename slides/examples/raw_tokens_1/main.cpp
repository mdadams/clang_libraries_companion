#include <iostream>
#include <format>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/Token.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

namespace ct = clang::tooling;

class RawTokenCollector : public clang::ASTFrontendAction {
public:
	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
	  clang::CompilerInstance&, llvm::StringRef inFile) override {
		llvm::outs() << std::format("processing source file {}\n",
		  std::string(inFile));
		return std::make_unique<clang::ASTConsumer>();
	}
	void ExecuteAction() override;
};

void RawTokenCollector::ExecuteAction() {
	clang::CompilerInstance& compInstance = getCompilerInstance();
	clang::SourceManager& sourceManager = compInstance.getSourceManager();
	clang::FileID fid = sourceManager.getMainFileID();
	clang::SourceLocation startLoc = sourceManager.getLocForStartOfFile(fid);
	clang::SourceLocation endLoc = sourceManager.getLocForEndOfFile(fid);
	clang::Lexer rawLexer(startLoc, compInstance.getLangOpts(),
	  sourceManager.getBufferData(fid).data(),
	  sourceManager.getCharacterData(startLoc),
	  sourceManager.getCharacterData(endLoc));
	while (true) {
		clang::Token token;
		rawLexer.LexFromRawLexer(token);
		if (token.is(clang::tok::eof)) {break;}
		llvm::outs() << std::format("token {}\n",
		  rawLexer.getSpelling(token, sourceManager,
		  compInstance.getLangOpts()));
	}
}

static llvm::cl::OptionCategory myToolCategory("raw-lexer-tool");

int main(int argc, const char** argv) {
	auto expectedParser = ct::CommonOptionsParser::create(argc, argv,
	  myToolCategory);
	if (!expectedParser) {
		llvm::errs() << std::format("Error: {}\n",
		  toString(expectedParser.takeError()));
		return 1;
	}
	ct::CommonOptionsParser &optionsParser = *expectedParser;
	ct::ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());
	return tool.run(ct::newFrontendActionFactory<RawTokenCollector>().get());
}
