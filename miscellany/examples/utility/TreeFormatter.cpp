#include <format>
#include "TreeFormatter.hpp"

//#define USE_LLVM_STREAM
#ifdef USE_LLVM_STREAM
#include <cstdint>
#include <llvm/Support/raw_ostream.h>
#else
#include <iostream>
#endif

#ifdef USE_LLVM_STREAM
using StreamTreeFormatter = TreeFormatter<llvm::raw_ostream>;
llvm::raw_ostream& getOutStream() {
	return llvm::outs();
}
#else
using StreamTreeFormatter = TreeFormatter<std::ostream>;
std::ostream& getOutStream() {
	return std::cout;
}
#endif

int main(int argc, char** argv) {
	std::string prefix;
	if (argc >= 2) {
		prefix = argv[1];
	}
	bool lastChild = false;
	if (argc >= 3) {
		lastChild = std::atoi(argv[2]);
	}
	int mode = (argc >= 4) ? std::atoi(argv[3]) : 0;
	std::cout << std::format("prefix {} lastChild {} mode {}\n",
	  prefix, lastChild, mode);
	StreamTreeFormatter formatter(getOutStream());
	if (!prefix.empty()) {
		formatter.setPrefix(prefix);
	}
	getOutStream() << std::format("mode {}\n", mode);
	if (mode) {
		for (int i = 0; i < 10; ++i) {
			formatter.down();
			formatter.up();
		}
	}
	formatter.down();
		formatter.addNode("Node A\nInfo", lastChild);
		formatter.down();
			formatter.addNode("Node AB\nInfo\nMore info", lastChild);
			formatter.down();
				formatter.addNode("Node ABC\nInfo\nMore info");
				formatter.down();
					formatter.addNode("Node ABCD\nInfo\nMore info", lastChild);
					formatter.down();
						formatter.addNode("Node ABCDE\nInfo\nMore info");
						formatter.addNode("Node ABCDF\nInfo\nMore info", lastChild);
					formatter.up();
				formatter.up();
				formatter.addNode("Node ABG\nInfo\nMore info");
				formatter.addNode("Node ABH\nInfo\nMore info");
				formatter.addNode("Node ABI");
				if (mode) {
					for (int i = 0; i < 10; ++i) {
						formatter.down();
						formatter.up();
					}
				}
				formatter.addNode("Node ABJ", lastChild);
				formatter.down();
					formatter.addNode("Node ABJK", lastChild);
					formatter.down();
						formatter.addNode("Node ABJKL", lastChild);
					formatter.up();
				formatter.up();
			formatter.up();
		formatter.up();
	formatter.up();

}
