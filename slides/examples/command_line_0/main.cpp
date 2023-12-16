#include <format>
#include <string>
#include <string_view>
#include "llvm/Support/CommandLine.h"

namespace lc = llvm::cl;

static lc::OptionCategory toolOpts("Tool options");
static lc::opt<std::string> logFile("log-file", lc::ZeroOrMore);
static lc::opt<bool> verbose("verbose", lc::Optional, lc::cat(toolOpts));
static lc::opt<int> debugLevel("debug-level", lc::Optional,
  lc::init(-1), lc::cat(toolOpts));
static lc::list<int> values("value", lc::ZeroOrMore, lc::cat(toolOpts));
static lc::alias valuesAlias("v", lc::aliasopt(values));
static lc::list<std::string> files(lc::Positional, lc::ZeroOrMore,
  lc::cat(toolOpts));

static void printVersion(llvm::raw_ostream& out) {out << "version 0.0.0\n";}

int main(int argc, char **argv) {
	lc::SetVersionPrinter(printVersion);
	lc::ParseCommandLineOptions(argc, argv,
	  "This program illustrates the use of the LLVM CommandLine API.");
	llvm::outs()
	  << std::format("log file: {}\n",
	  !logFile.empty() ? std::string_view(logFile) : "---")
	  << std::format("debug level: {}\n", static_cast<bool>(debugLevel))
	  << std::format("verbose: {}\n", static_cast<bool>(verbose));
	for (auto i : values) {llvm::outs() << std::format("value: {}\n", i);}
	for (auto i : files) {llvm::outs() << std::format("file: {}\n", i);}
}
