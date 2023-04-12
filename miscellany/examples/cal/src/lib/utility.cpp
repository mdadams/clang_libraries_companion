#include <format>
#include <string>
#include <vector>
#include <iostream>
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

std::string getClangProgramPath() {
	auto path = boost::process::search_path(boost::filesystem::path("clang++"));
	return path.string();
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
	while (proc.running() && std::getline(is, line) && !line.empty()) {
		ss << line << '\n';
		if (!ss) {
			okay = false;
			break;
		}
	}
	proc.wait();
	int exitStatus = proc.exit_code();
	if (exitStatus) {
		return "";
	}
	std::string version;
	std::string dummy;
	ss >> dummy >> dummy >> version;
	if (!ss) {
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
	std::string clangVersion = getClangVersion(clangProgramPath.string());
	if (clangProgramPath.empty() || clangVersion.empty()) {
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
	return found ? path.string() : "";
}

} // namespace cal
