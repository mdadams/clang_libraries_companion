#include <cstdlib>
#include <format>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/process/environment.hpp>
#include <boost/process.hpp>
#include <boost/process/search_path.hpp>
#include <boost/tokenizer.hpp>
#include "cal/main.hpp"

namespace bf = boost::filesystem;
namespace bp = boost::process;

namespace cal {

/****************************************************************************\
Semantic Version Numbering
\****************************************************************************/

// Semantic version number
struct Version {
	int major;
	int minor;
	int patch;
};

bool parseVersion(const std::string& versionString, Version& version)
{
	boost::char_separator<char> sep(".");
	boost::tokenizer<boost::char_separator<char>> tokenizer(versionString,
	  sep);
	auto i = tokenizer.begin();
	version = {0, 0, 0};
	if (i != tokenizer.end()) {
		version.major = std::atoi(i->c_str());
		++i;
		if (i != tokenizer.end()) {
			version.minor = std::atoi(i->c_str());
			++i;
			if (i != tokenizer.end()) {
				version.patch = std::atoi(i->c_str());
				++i;
				if (i != tokenizer.end()) {
					return false;
				}
			}
		}
	}
	return true;
}

/****************************************************************************\
Code Pretty Printing
\****************************************************************************/

unsigned int getLineNumSize(unsigned int maxLineNo)
{
	unsigned int count = 1;
	while (maxLineNo >= 10) {
		maxLineNo /= 10;
		++count;
	}
	return count;
}

std::string addLineNumbers(const std::string& text, unsigned int startLineNo,
  unsigned int startColumnNo, bool lineHeader, bool columnHeader)
{
	std::string result;

	if (text.empty()) {
		return "[empty text]\n";
	}

	unsigned int maxLineNo = startLineNo;
	unsigned int columnNo = startColumnNo;
	unsigned int minColumnNo = startColumnNo;
	unsigned int maxColumnNo = startColumnNo;

	for (auto i = text.begin(); i != text.end(); ++i) {
		if (*i == '\n') {
			++maxLineNo;
			columnNo = 1;
		} else {
		minColumnNo = std::min(minColumnNo, columnNo);
		maxColumnNo = std::max(maxColumnNo, columnNo);
			++columnNo;
		}
	}
	unsigned int lineNumSize = getLineNumSize(maxLineNo);
	unsigned int columnNumSize = getLineNumSize(maxColumnNo);

	if (columnHeader) {
		constexpr char digits[] = "0123456789";
		if (columnNumSize >= 2) {
			if (lineHeader) {
				result += std::string(lineNumSize + 2, ' ');
			}
			for (unsigned int i = minColumnNo; i <= maxColumnNo; ++i) {
				result += digits[(i / 10) % 10];
			}
			result += '\n';
		}
		if (lineHeader) {
			result += std::string(lineNumSize + 2, ' ');
		}
		for (unsigned int i = minColumnNo; i <= maxColumnNo; ++i) {
			result += digits[i % 10];
		}
		result += '\n';
		if (lineHeader) {
			result += std::string(lineNumSize + 2, ' ');
		}
		result += std::string(maxColumnNo - minColumnNo + 1, '-');
		result += '\n';
	}

	columnNo = minColumnNo;
	auto i = text.begin();
	for (unsigned int lineNo = startLineNo; lineNo <= maxLineNo; ++lineNo) {
		if (lineHeader) {
			result += std::format("{:{}d}: ", lineNo, lineNumSize);
		}
		if (lineNo == startLineNo) {
			result += std::string(startColumnNo - minColumnNo, ' ');
		}
		while (i != text.end()) {
			char c = *i++;
			if (c == '\n') {
				columnNo = minColumnNo;
				//result += '$';
				break;
			}
			result += c;
			++columnNo;
		}
		result += '\n';
	}

	return result;
}

/****************************************************************************\
Clang Include Directory
\****************************************************************************/

#define CAL_HANDLE_CCACHE
std::string getClangProgramPath()
{
#if defined(CAL_HANDLE_CCACHE)
	std::vector<bf::path> searchPath = boost::this_process::path();
	auto clangPath = boost::process::search_path(bf::path("clang++"),
	  searchPath);
	if (clangPath.has_parent_path()) {
		bf::path dirPath = clangPath.parent_path();
		bf::path dirName = dirPath.filename();
		if (dirName == "ccache") {
			std::vector<bf::path> newSearchPath;
			auto i = searchPath.begin();
			for (auto i = searchPath.begin(); i != searchPath.end(); ++i) {
				if (*i != dirPath) {
					newSearchPath.push_back(*i);
				}
			}
			clangPath = boost::process::search_path(bf::path("clang++"),
			  newSearchPath);
		}
	}
	return clangPath.string();
#else
	auto path = boost::process::search_path(
	  bf::path("clang++"));
	return path.string();
#endif
}

std::string getClangResourceDirPath(const std::string& clangProgramPath)
{
	std::string realClangProgramPath = clangProgramPath.empty() ?
	  getClangProgramPath() : clangProgramPath;
	if (realClangProgramPath.empty()) {
		return "";
	}
	bp::ipstream is;
	std::vector<std::basic_string<char>> args;
	args.emplace_back("-print-resource-dir");
	bp::child proc(realClangProgramPath, bp::args(args),
	  bp::std_out > is, bp::std_err > "/dev/null");
	bool firstLine = true;
	std::string resourceDir;
	std::string line;
	while (std::getline(is, line) && !line.empty()) {
		if (firstLine) {
			resourceDir = line;
			firstLine = false;
		}
	}
	proc.wait();
	int exitStatus = proc.exit_code();
	if (exitStatus) {
#if defined(CAL_DEBUG)
		std::cerr << std::format("clang exit status {}\n", exitStatus);
#endif
		return "";
	}
	return resourceDir;
}

std::string getClangVersion(const std::string& clangProgramPath)
{
	bp::ipstream is;
	std::vector<std::basic_string<char>> args;
	args.emplace_back("--version");
	bp::child proc(clangProgramPath, bp::args(args),
	  bp::std_out > is, bp::std_err > "/dev/null");
	std::stringstream ss;
	std::string line;
	bool okay = true;
	while (std::getline(is, line) && !line.empty()) {
		ss << line << '\n';
		if (!ss) {
			okay = false;
			break;
		}
	}
	proc.wait();
	int exitStatus = proc.exit_code();
	if (exitStatus) {
#if defined(CAL_DEBUG)
		std::cerr << std::format("clang exit status {}\n", exitStatus);
#endif
		return "";
	}
	std::string version;
	std::string dummy;
#if defined(CAL_DEBUG)
	std::string buffer = ss.str();
#endif
	ss >> dummy >> dummy >> version;
	if (!ss) {
#if defined(CAL_DEBUG)
		std::cerr << "stream in failed state\n";
		std::cerr << std::format("stringstream: {}\n", buffer);
#endif
		version = "";
	}
	return version;
}

std::string getClangIncludeDirPath(const std::string& clangProgramPath)
{
	std::string realClangProgramPath = clangProgramPath.empty() ?
	  getClangProgramPath() : clangProgramPath;
	if (realClangProgramPath.empty()) {
		return "";
	}
	std::string resourceDir = getClangResourceDirPath(realClangProgramPath);
	if (resourceDir.empty()) {
		return "";
	}
	return (bf::path{resourceDir} / "include").string();
}

} // namespace cal
