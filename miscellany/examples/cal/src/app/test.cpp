#define CAL_INTERNAL // This line must precede any inclusion of CAL headers.
#include <cal/main.hpp>
#include <iostream>
#include <format>
#include <cassert>
#include <cstdlib>

int main(int argc, char** argv)
{
	std::string clangProgramPath = cal::getClangProgramPath();
	std::cout << std::format("clang program path: {}\n",
	  clangProgramPath);
#if defined(CAL_INTERNAL)
	std::string clangVersion = cal::getClangVersion(clangProgramPath);
	std::cout << std::format("clang version: {}\n",
	  clangVersion);
#endif
	std::string clangResDir = cal::getClangResourceDirPath();
	std::cout << std::format("clang resource dir: {}\n",
	  clangResDir);
	assert(cal::getClangResourceDirPath() ==
	  cal::getClangResourceDirPath(clangProgramPath));
	std::string clangIncDir = cal::getClangIncludeDirPath();
	std::cout << std::format("clang include dir: {}\n",
	  clangIncDir);
	assert(cal::getClangIncludeDirPath() ==
	  cal::getClangIncludeDirPath(clangProgramPath));
	if (clangIncDir.empty()) {std::abort();}
	assert(!clangIncDir.empty());
}
