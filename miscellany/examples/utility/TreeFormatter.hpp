#ifndef TreeFormatter_hpp
#define TreeFormatter_hpp

#include <cassert>
#include <deque>
#include <ranges>
#include <string>
#include <string_view>

template<class Stream>
class TreeFormatter {
private:

	struct StackEntry {
		StackEntry(const std::string& prefix_, int childNo_, bool lastChild_) :
		  prefix(prefix_), childNo(childNo_), lastChild(lastChild_) {}
		std::string prefix;
		int childNo;
		bool lastChild;
	};

public:

	TreeFormatter() : out_(nullptr) {
		initialize();
	}

	TreeFormatter(Stream& out) : out_(&out) {
		initialize();
	}

	void initialize() {
		stack_ = {};
		rootNode_ = true;
		curLevel_ = -1;
		curPrefix_ = "";
		curChildNo_ = 0;
		curLastChild_ = true;
		flushLeft_ = true;
	}

	void setOutput(Stream& out) {
		out_ = &out;
	}

	void setPrefix(const std::string& prefix) {
		globalPrefix_ = prefix;
	}

	void setFlushLeft(bool flushLeft) {
		flushLeft_ = flushLeft;
	}

	void down() {
		// Note: Perhaps, it is better to allow the condition in the
		// following assertion to be violated.
		// assert(curChildNo_ > 0 || curLevel_ < 0);
		if (!(curChildNo_ > 0 || curLevel_ < 0)) {
			addNode(std::string("missing node (automatically injected)\n") +
			  std::format("level {}; childNo {}", curLevel_, curChildNo_));
			out_->flush();
		}
		stack_.push_back(StackEntry(curPrefix_, curChildNo_, curLastChild_));
		std::string prefix = curPrefix_;
		if (curLevel_ > 0 || (curLevel_ == 0 && !flushLeft_)) {
			prefix += curLastChild_ ? "    " : "│   ";
		}
		curPrefix_ = prefix;
		curChildNo_ = 0;
		curLastChild_ = false;
		++curLevel_;
	}

	void up() {
		StackEntry& entry = stack_.back();
		curPrefix_ = entry.prefix;
		curChildNo_ = entry.childNo;
		curLastChild_ = entry.lastChild;
		stack_.pop_back();
		--curLevel_;
	}

	template <std::ranges::range R>
	void addNodeLines(const R& lines, bool lastChild = false) {
		std::string buffer;
		bool first = true;
		for (const std::string& i : lines) {
			if (!first) {buffer += '\n';}
			buffer += i;
			first = false;
		}
		addNode(buffer, lastChild);
	}

	void addNode(const std::string& s, bool lastChild = false) {
		assert(curLevel_ >= 0);
		assert(!curLastChild_);
		curLastChild_ = lastChild;
		if (!rootNode_) {
			*out_ << getFullPrefix() << "│\n";
		}
		std::string leader;
		if (!(rootNode_ && flushLeft_)) {
			leader = lastChild ? "└── " : "├── ";
		}
		*out_ << getFullPrefix() << leader;
		if (!(rootNode_ && flushLeft_)) {
			leader = lastChild ? "    " : "│   ";
		}
		std::string_view adjusted(s.begin(),
		  s.ends_with('\n') ? s.end() - 1 : s.end());
		for (auto c : adjusted) {
			if (c == '\n') {
				*out_ << "\n" << getFullPrefix() << leader;
			} else {
				*out_ << c;
			}
		}
		*out_ << "\n";
		++curChildNo_;
		rootNode_ = false;
	}

	int getCurChildNo() const {
		return curChildNo_;
	}

private:

	std::string getFullPrefix() const {
		return globalPrefix_ + curPrefix_;
	}

	Stream* out_;

	std::deque<StackEntry> stack_;

	bool rootNode_;
	int curLevel_;

	std::string curPrefix_;
	int curChildNo_;
	bool curLastChild_;;

	bool flushLeft_;
	std::string globalPrefix_;
};

#endif
