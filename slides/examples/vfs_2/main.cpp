#include <memory>
#include <string>
#include <vector>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/VirtualFileSystem.h>

namespace ct = clang::tooling;
namespace lc = llvm::cl;
namespace cam = clang::ast_matchers;

cam::DeclarationMatcher getMatcher() {
	using namespace cam;
	return functionDecl(isExpansionInMainFile()).bind("func");
}

class FuncDeclHandler : public cam::MatchFinder::MatchCallback {
public:
	void run(const cam::MatchFinder::MatchResult& result) override {
		if (const clang::FunctionDecl *funcDecl =
		  result.Nodes.getNodeAs<clang::FunctionDecl>("func")) {
			llvm::outs() << std::format("found declaration for function {}\n",
			  funcDecl->getNameAsString());
		}
	}
};

std::unique_ptr<ct::CompilationDatabase> makeCompDatabase(
  const std::vector<std::string>& options) {
	std::string buffer;
	for (auto i : options) {buffer += i + "\n";}
	std::string errText;
	return ct::FixedCompilationDatabase::loadFromBuffer(".", buffer, errText);
}

static lc::opt<std::string> clangIncDir("clang-include-dir", lc::Required);
static lc::list<std::string> sourcePaths(lc::Positional, lc::ZeroOrMore);

std::string headerSource = R"(
#ifndef HG2G_MAIN_HPP
#define HG2G_MAIN_HPP
namespace hg2g {
	constexpr auto get_answer() {
		return 42;
	}
}
#endif
)";

std::string appSource = R"(
#include <iostream>
#include "/usr/include/hg2g/main.hpp"
int main() {
	std::cout << "The answer is " << hg2g::get_answer() << "\n";
}
)";

int main(int argc, const char** argv) {
	lc::ParseCommandLineOptions(argc, argv, "VFS example");
	std::vector<std::string> compOptions{
	  std::format("-I{}", std::string(clangIncDir)), "-std=c++20",
	};
	auto compDatabase = makeCompDatabase(compOptions);
	if (!compDatabase) {
		llvm::errs() << "cannot create compilation database\n";
		return 1;
	}
	llvm::IntrusiveRefCntPtr<llvm::vfs::OverlayFileSystem> overlayFileSys(
		new llvm::vfs::OverlayFileSystem(llvm::vfs::getRealFileSystem()));
	llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> memFileSys(
		new llvm::vfs::InMemoryFileSystem);
	overlayFileSys->pushOverlay(memFileSys);
	memFileSys->addFile("/virtual.cpp", 0,
	  llvm::MemoryBuffer::getMemBuffer(appSource));
	memFileSys->addFile("/usr/include/hg2g/main.hpp", 0,
	  llvm::MemoryBuffer::getMemBuffer(headerSource));
	FuncDeclHandler matchCallback;
	cam::MatchFinder matchFinder;
	matchFinder.addMatcher(getMatcher(), &matchCallback);
	ct::ClangTool tool(*compDatabase, sourcePaths,
	  std::make_shared<clang::PCHContainerOperations>(), overlayFileSys,
	  nullptr);
	return tool.run(ct::newFrontendActionFactory(&matchFinder).get());
}
