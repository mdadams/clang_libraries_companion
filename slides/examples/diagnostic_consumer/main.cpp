#include <format>
#include <map>
#include <string>
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

namespace ct = clang::tooling;

std::string locationToString(const clang::SourceManager& sourceManager,
  clang::SourceLocation sourceLoc) {
	return std::format("{}:{}:{}", sourceManager.getFilename(sourceLoc),
	  sourceManager.getSpellingLineNumber(sourceLoc),
	  sourceManager.getSpellingColumnNumber(sourceLoc));
}

std::string levelToString(clang::DiagnosticsEngine::Level level) {
	const std::map<clang::DiagnosticsEngine::Level, std::string> lut{
	  {clang::DiagnosticsEngine::Level::Error, "error"},
	  {clang::DiagnosticsEngine::Level::Fatal, "fatal error"},
	};
	auto i = lut.find(level);
	return i != lut.end() ? i->second : "unknown";
}

class MyDiagnosticConsumer : public clang::DiagnosticConsumer {
public:
	MyDiagnosticConsumer() : errCount_(0) {}
	void HandleDiagnostic(clang::DiagnosticsEngine::Level diagLevel,
	  const clang::Diagnostic& info) override {
		clang::SourceManager* sm = info.hasSourceManager() ?
		  &info.getSourceManager() : nullptr;
		if (diagLevel == clang::DiagnosticsEngine::Level::Error ||
		  diagLevel == clang::DiagnosticsEngine::Level::Fatal) {
			if (sm) {
				llvm::errs() << std::format("{} at {}\n",
				  levelToString(diagLevel), locationToString(*sm,
				  info.getLocation()));
				++errCount_;
			} else {
				llvm::errs() << std::format("{}\n", levelToString(diagLevel));
			}
		}
	}
	unsigned long getErrCount() const {return errCount_;}
private:
	unsigned long errCount_;
};

static llvm::cl::OptionCategory toolOptions("Tool Options");

int main(int argc, char** argv) {
	auto expectedOptionsParser = ct::CommonOptionsParser::create(argc,
	  const_cast<const char**>(argv), toolOptions);
	if (!expectedOptionsParser) {
		llvm::errs() << llvm::toString(expectedOptionsParser.takeError());
		return 1;
	}
	ct::CommonOptionsParser& optionsParser = *expectedOptionsParser;
	ct::ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());
	MyDiagnosticConsumer diagnosticConsumer;
	tool.setDiagnosticConsumer(&diagnosticConsumer);
	int status = tool.run(
	  ct::newFrontendActionFactory<clang::SyntaxOnlyAction>().get());
	unsigned long errCount = diagnosticConsumer.getErrCount();
	if (errCount) {
		llvm::errs() << std::format("{} error(s) occurred\n", errCount);
	}
	return (!status && !errCount) ? 0 : 1;
}
