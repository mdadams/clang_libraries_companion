#include <format>
#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/Token.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

namespace ct = clang::tooling;

class DumpTokensPPAction : public clang::ASTFrontendAction {
public:
	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
	  clang::CompilerInstance&, llvm::StringRef inFile) override {
		llvm::outs() << std::format("processing source file {}\n",
		  std::string(inFile));
		return std::make_unique<clang::ASTConsumer>();
	}
	void ExecuteAction() override;
};

void DumpTokensPPAction::ExecuteAction() {
	clang::CompilerInstance& compInstance = getCompilerInstance();
	clang::Preprocessor& preproc = compInstance.getPreprocessor();
	preproc.EnterMainSourceFile();
    llvm::outs() << std::format("{:20} {}\n", "Kind", "Spelling");
	while (true) {
		clang::Token token;
		preproc.Lex(token);
		if (token.is(clang::tok::eof)) {break;}
		llvm::outs() << std::format("{:20} {}\n", token.getName(),
		  preproc.getSpelling(token));
	}
}

static llvm::cl::OptionCategory myToolCategory("dump-tokens options");

int main(int argc, const char** argv) {
	auto expectedParser = ct::CommonOptionsParser::create(argc, argv,
	  myToolCategory);
	if (!expectedParser) {
		llvm::errs() << "Error while parsing options.\n";
		return 1;
	}
	ct::CommonOptionsParser& optionsParser = expectedParser.get();
	ct::ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());
	return tool.run(ct::newFrontendActionFactory<DumpTokensPPAction>().get());
}
