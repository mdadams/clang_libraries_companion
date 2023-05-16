#define CAL_INTERNAL // This line must precede any inclusion of CAL headers.
#include <cal/main.hpp>
#include <iostream>
#include <format>
#include <cassert>
#include <cstdlib>

int main(int argc, char** argv)
{
#if defined(CAL_INTERNAL)
	std::string clangProgramPath = cal::getClangProgramPath();
	std::cout << std::format("clang program path: {}\n",
	  clangProgramPath);
	std::string clangVersion = cal::getClangVersion(clangProgramPath);
	std::cout << std::format("clang version: {}\n",
	  clangVersion);
#endif
	std::string clangIncDir = cal::getClangIncludeDirPathName();
	std::cout << std::format("clang include dir: {}\n",
	  clangIncDir);
	if (clangIncDir.empty()) {std::abort();}
	assert(!clangIncDir.empty());
}
