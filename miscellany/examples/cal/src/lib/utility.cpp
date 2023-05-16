#include <format>
#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/process/environment.hpp>
#include <boost/process.hpp>
#include <boost/process/search_path.hpp>
#include "cal/main.hpp"

namespace bf = boost::filesystem;
namespace bp = boost::process;

namespace cal {

unsigned int getLineNumSize(unsigned int maxLineNo) {
	unsigned int count = 1;
	while (maxLineNo >= 10) {
		maxLineNo /= 10;
		++count;
	}
	return count;
}

std::string addLineNumbers(const std::string& text, unsigned int startLineNo,
  unsigned int startColumnNo, bool lineHeader, bool columnHeader) {
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

#define CAL_HANDLE_CCACHE
std::string getClangProgramPath() {
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

std::string getClangVersion(const std::string& pathname) {
	bp::ipstream is;
	std::vector<std::basic_string<char>> args;
	args.emplace_back("--version");
	bp::child proc(pathname, bp::args(args),
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

std::string getClangIncludeDirPathName() {
	const std::vector<std::string> subDirs{
		"lib64",
		"lib",
	};
	bf::path clangProgramPath = getClangProgramPath();
	if (clangProgramPath.empty()) {
#if defined(CAL_DEBUG)
		std::cerr << "getClangProgramPath failed\n";
#endif
		return "";
	}
	std::string clangVersion = getClangVersion(clangProgramPath.string());
	if (clangVersion.empty()) {
#if defined(CAL_DEBUG)
		std::cerr << "getClangVersion failed\n";
#endif
		return "";
	}
	bf::path prefix = clangProgramPath.parent_path().parent_path();
	bf::path path;
	bool found = false;
	for (auto subDir : subDirs) {
		path = prefix;
		path /= subDir;
		path /= bf::path("clang") /= bf::path(clangVersion) /= bf::path("include");
		auto status = bf::status(path);
		if (bf::is_directory(status)) {
			found = true;
			break;
		}
	}
	if (!found) {
#if defined(CAL_DEBUG)
		std::cerr << "getClangIncludeDirPathName failed: search failed\n";
#endif
	}
	assert(!path.string().empty());
	return found ? path.string() : "";
}

} // namespace cal
