#include <format>
#include <string>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

using namespace std::literals;
namespace lc = llvm::cl;
namespace ct = clang::tooling;

static lc::OptionCategory toolOptionCat("Tool Options");
static lc::extrahelp CommonHelp(ct::CommonOptionsParser::HelpMessage);
static lc::extrahelp MoreHelp(
	"This tool does not actually do anything useful.\n"
	"Life is full of disappointments.  Get over it.\n"
);
lc::opt<std::string> outFile("o", lc::desc("Output file"),
  lc::value_desc("output_file"), lc::cat(toolOptionCat));
lc::opt<bool> verbose("verbose", lc::desc("Enable verbose output."),
  lc::cat(toolOptionCat));
lc::alias verbose2("v", lc::desc("Alias for -verbose"), lc::aliasopt(verbose));
lc::opt<bool> foobar("foobar", lc::desc("Enable experimental features."),
  lc::Hidden);
lc::opt<std::string> opName(lc::Positional, lc::Required,
  lc::desc("Operation to perform."), lc::value_desc("op_name"),
  lc::cat(toolOptionCat));

int main(int argc, const char **argv) {
	llvm::Expected<ct::CommonOptionsParser> expectedOptionsParser(
	  ct::CommonOptionsParser::create(argc, argv, toolOptionCat));
	if (!expectedOptionsParser) {
		llvm::errs() << std::format("Unable to create option parser ({}).\n",
		  llvm::toString(std::move(expectedOptionsParser.takeError())));
		return 1;
	}
	ct::CommonOptionsParser& optionsParser = *expectedOptionsParser;
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
