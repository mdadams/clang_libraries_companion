#include <format>
#include <memory>
#include <ranges>
#include <string>
#include <vector>
#include <clang/DependencyScanning/DependencyScanningService.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Tooling/ArgumentsAdjusters.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/DependencyScanningTool.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>

namespace lc = llvm::cl;
namespace cd = clang::dependencies;
namespace ct = clang::tooling;

static lc::opt<std::string> clCdbPath("cdb-file",
  lc::desc("compilation database path"), lc::Required);
static lc::opt<std::string> clResourceDir("resource-dir",
  lc::desc("Clang resource directory"), lc::Required);
static lc::opt<bool> clShowHeaders("headers",
  lc::desc("show header dependencies"), lc::init(false));
static lc::opt<bool> clShowModules("modules",
  lc::desc("show module dependencies"), lc::init(false));
static lc::opt<bool> clCanonPath("canonical-paths",
  lc::desc("canonicalize paths"), lc::init(false));
static lc::list<std::string> clSourcePaths(lc::Positional,
  lc::desc("source paths"), lc::ZeroOrMore);

std::string getCanonPath(const std::string& path,
  const std::string& workdir_path) {
    llvm::SmallString<256> absPath;
    if (llvm::sys::path::is_relative(path)) {
        absPath = workdir_path;
        llvm::sys::path::append(absPath, path);
    } else {absPath = path;}
    llvm::SmallString<256> canonPath;
    if (llvm::sys::fs::real_path(absPath, canonPath)) {
        llvm::sys::path::remove_dots(absPath, true);
        return absPath.str().str();
    }
    return canonPath.str().str();
}

template<typename Range>
std::vector<ct::CompileCommand> getCompCommandsForSourceFiles(
  const ct::CompilationDatabase& cdb, const Range& paths) {
	std::vector<ct::CompileCommand> commands;
	if (!paths.empty()) {
		for (const auto& path : paths) {
			auto tmp = cdb.getCompileCommands(path);
			commands.insert(commands.end(), tmp.begin(), tmp.end());
		}
	} else {commands = cdb.getAllCompileCommands();}
	return commands;
}

ct::CommandLineArguments stripModuleOpts(
  const ct::CommandLineArguments &args) {
	auto filtered = args | std::views::filter([](const std::string& arg) {
		return arg != "-fmodules-ts" && arg != "-fdeps-format=p1689r5" &&
		!arg.starts_with("-fmodule-mapper=") &&
		!arg.starts_with("-fdeps-file=") && !arg.starts_with("-fdeps-target=");
	});
	return ct::CommandLineArguments(filtered.begin(), filtered.end());
}

ct::CommandLineArguments adjustArgs(const ct::CommandLineArguments& args) {
	auto adjustedArgs = args;
	adjustedArgs = stripModuleOpts(adjustedArgs);
	adjustedArgs.push_back("-resource-dir");
	adjustedArgs.push_back(clResourceDir);
	return adjustedArgs;
}

llvm::Error processCompCommand(ct::DependencyScanningTool& tool,
  ct::CompileCommand compCommand) {
	ct::CommandLineArguments args = adjustArgs(compCommand.CommandLine);
	clang::DiagnosticOptions diagOpts;
	clang::TextDiagnosticPrinter diagConsumer(llvm::errs(), diagOpts);
	llvm::DenseSet<cd::ModuleID> alreadySeen;
	auto maybeTuDeps = tool.getTranslationUnitDependencies(args,
	  compCommand.Directory, diagConsumer, alreadySeen, nullptr);
	if (!maybeTuDeps) {
		return llvm::createStringError(llvm::inconvertibleErrorCode(),
		  "cannot get dependencies");
	}
	const cd::TranslationUnitDeps& tuDeps = *maybeTuDeps;
	std::string mainSourceCanonPath = getCanonPath(compCommand.Filename,
	  compCommand.Directory);
	llvm::outs() << std::format("- path: {}\n", clCanonPath ?
	  mainSourceCanonPath : compCommand.Filename);
	if (!tuDeps.ID.ModuleName.empty() || !tuDeps.NamedModuleDeps.empty()) {
		llvm::outs() << std::format("  context_hash: {}\n",
		  tuDeps.ID.ContextHash);
	}
	if (clShowModules) {
		if (!tuDeps.ID.ModuleName.empty()) {
			llvm::outs() << std::format("  provides:\n    - name: {}\n",
			  tuDeps.ID.ModuleName);
		}
		if (!tuDeps.NamedModuleDeps.empty()) {
			llvm::outs() << "  requires:\n";
			for (const std::string& modName : tuDeps.NamedModuleDeps) {
				llvm::outs() << std::format("    - name: {}\n", modName);
			}
		}
	}
	if (clShowHeaders) {
		bool first = true;
		for (const std::string& path : tuDeps.FileDeps) {
			std::string canonPath = getCanonPath(path, compCommand.Directory);
			if (mainSourceCanonPath != canonPath) {
				if (first) {llvm::outs() << "  headers:\n";}
				llvm::outs() << std::format("    - path: {}\n", clCanonPath ?
				  canonPath : path);
				first = false;
			}
		}
	}
	return llvm::Error::success();
}

int main(int argc, char **argv) {
	lc::ParseCommandLineOptions(argc, argv, "dependency scanner\n");
	if (!clShowHeaders && !clShowModules) {
		clShowHeaders = clShowModules = true;
	}
	std::string errMessage;
	auto cdb = ct::JSONCompilationDatabase::loadFromFile(
	    clCdbPath, errMessage, ct::JSONCommandLineSyntax::AutoDetect);
	if (!cdb) {
		llvm::errs() << std::format("error loading CDB: {}\n", errMessage);
		return 1;
	}
	cd::DependencyScanningService service(
	  cd::ScanningMode::CanonicalPreprocessing, cd::ScanningOutputFormat::Full);
	ct::DependencyScanningTool tool(service);
	auto compCommands = getCompCommandsForSourceFiles(*cdb, clSourcePaths);
	int failCount = 0;
	for (const auto& command : compCommands) {
		if (auto err = processCompCommand(tool, command)) {
			llvm::errs() << llvm::toString(std::move(err)) << "\n";
			++failCount;
		}
	}
	return failCount == 0 ? 0 : 1;
}
