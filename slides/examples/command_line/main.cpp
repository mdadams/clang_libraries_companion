#include <format>
#include <string>
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

using namespace std::literals;

static llvm::cl::OptionCategory toolOptionCat("Tool Options");
static llvm::cl::extrahelp
  CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);
static llvm::cl::extrahelp MoreHelp(
	"This tool does not actually do anything useful.\n"
	"Life is full of disappointments.  Get over it.\n"
);
llvm::cl::opt<std::string> outFile(
  "o", llvm::cl::desc("Output file"), llvm::cl::value_desc("output_file"),
  llvm::cl::cat(toolOptionCat));
llvm::cl::opt<bool> verbose("verbose",
  llvm::cl::desc("Enable verbose output."), llvm::cl::cat(toolOptionCat));
llvm::cl::alias verbose2("v", llvm::cl::desc("Alias for -verbose"),
  llvm::cl::aliasopt(verbose));
llvm::cl::opt<bool> foobar("foobar",
  llvm::cl::desc("Enable experimental features."), llvm::cl::Hidden);
llvm::cl::opt<std::string> opName(llvm::cl::Positional, llvm::cl::Required,
  llvm::cl::desc("Operation to perform."),
  llvm::cl::value_desc("op_name"), llvm::cl::cat(toolOptionCat));

int main(int argc, const char **argv) {
	llvm::Expected<clang::tooling::CommonOptionsParser> expectedOptionsParser(
	  clang::tooling::CommonOptionsParser::create(argc, argv, toolOptionCat));
	if (!expectedOptionsParser) {
		llvm::errs() << std::format("Unable to create option parser ({}).\n",
		  llvm::toString(std::move(expectedOptionsParser.takeError())));
		return 1;
	}
	clang::tooling::CommonOptionsParser& optionsParser = *expectedOptionsParser;
	llvm::outs()
	  << std::format("verbose: {}\n", static_cast<bool>(verbose))
	  << std::format("foobar: {}\n", static_cast<bool>(foobar))
	  << std::format("operation: {}\n",
	    !opName.empty() ? opName : "(null)"s)
	  << std::format("output file: {}\n",
	    !outFile.empty() ? outFile : "(null)"s);
	llvm::outs() << std::format("number of compilation database entries: {}\n",
	  optionsParser.getCompilations().getAllCompileCommands().size());
	llvm::outs() << "source paths:\n";
	for (auto path : optionsParser.getSourcePathList()) {
		llvm::outs() << std::format("    {}\n", path);
	}
	return 0;
}
