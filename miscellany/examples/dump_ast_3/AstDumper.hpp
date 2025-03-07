#ifndef AstDumper_hpp
#define AstDumper_hpp

#include <deque>
#include <format>
#include <functional>
#include <ranges>
#include <string>

#include <clang/AST/APValue.h>
#include <clang/AST/ASTNodeTraverser.h>
#include <clang/AST/Comment.h>
#include <clang/Basic/SourceManager.h>
#include <llvm/Support/raw_ostream.h>

#include "TreeFormatter.hpp"
#include "clang_utility_1.hpp"

class AstNodeDumper {
public:

	friend class AstDumper;

	using BigInt = long long;
	using DoAddChildQueue = std::deque<std::function<void()>>;
	struct NodeStackEntry {
		NodeStackEntry(BigInt nodeId_, int level_) :
		  nodeId(nodeId_), level(level_), numChildren(0) {}
		BigInt nodeId;
		int level;
		int numChildren;
		DoAddChildQueue doAddChildQueue;
	};
	using NodeStack = std::deque<NodeStackEntry>;

	AstNodeDumper(clang::SourceManager& sourceManager,
	  const clang::LangOptions& langOpts, llvm::raw_ostream& out,
	  bool enableLastChild, bool flushLeft) :
	  sourceManager_(&sourceManager), langOpts_(&langOpts), out_(&out) {
		treeFormatter_.setOutput(out);
		treeFormatter_.setFlushLeft(flushLeft);
		if (logLevel_ >= 1) {
			treeFormatter_.setPrefix("TREE: ");
		}
		enableLastChild_ = enableLastChild;
		lastChild_ = enableLastChild_;
	}

	AstNodeDumper(const AstNodeDumper&) = delete;
	AstNodeDumper& operator=(const AstNodeDumper&) = delete;

	BigInt getCurNodeId() const {
		return nodeStack_.back().nodeId;
	}
	BigInt getCurChildNo() const {
		return nodeStack_.back().numChildren - nodeStack_.back().doAddChildQueue.size();
	}
	BigInt getCurParentNodeId() const {
		return nodeStack_.size() >= 2 ?
		  nodeStack_[nodeStack_.size() - 2].nodeId : -1;
	}
	BigInt getCurLevel() const {
		return nodeStack_.back().level;
	}

	template <typename... Args>
	void logf(int level, std::format_string<Args...> fmt, Args&&... args) {
		if (logLevel_ >= level) {
			*out_ << std::format(fmt, std::forward<Args>(args)...);
		}
	}

	template <typename Fn> void AddChild(Fn doAddChild) {
		return AddChild("", doAddChild);
	}
	template <typename Fn> void AddChild(llvm::StringRef label, Fn doAddChild) {
		if (topLevel_) {
			topLevel_ = false;
			nodeStack_.push_back(NodeStackEntry(makeNodeId(), 0));
			treeFormatter_.down();
			enableTraverse_ = false;
			BigInt oldVisitCount = visitCount_;
			doAddChild();
			assert(visitCount_ == oldVisitCount + 1);
			if (!enableTraverse_) {
				NodeStackEntry& entry = nodeStack_.back();
				DoAddChildQueue& doAddChildQueue = entry.doAddChildQueue;
				//llvm::outs() << "dropping children\n";
				doAddChildQueue.clear();
			}
			while (!nodeStack_.empty()) {
				NodeStackEntry& entry = nodeStack_.back();
				DoAddChildQueue& doAddChildQueue = entry.doAddChildQueue;
				if (!doAddChildQueue.empty()) {
					std::function<void()> doAddChild =
					  std::move(doAddChildQueue.front());
					doAddChildQueue.pop_front();
					if (enableLastChild_) {
						lastChild_ = doAddChildQueue.empty();
					} else {
						lastChild_ = false;
					}
					nodeStack_.push_back(NodeStackEntry(makeNodeId(),
					  entry.level + 1));
					treeFormatter_.down();
					enableTraverse_ = false;
					BigInt oldVisitCount = visitCount_;
					doAddChild();
					assert(visitCount_ == oldVisitCount + 1);
					if (!enableTraverse_) {
						doAddChildQueue.clear();
					}
				} else {
					nodeStack_.pop_back();
					treeFormatter_.up();
				}
			}
		} else {
			NodeStackEntry& entry = nodeStack_.back();
			DoAddChildQueue& doAddChildQueue = entry.doAddChildQueue;
			doAddChildQueue.push_back(std::move(doAddChild));
			++entry.numChildren;
		}
	}

	std::vector<std::string> makeDesc(const std::string& label,
	  clang::SourceRange sourceRange = {}) const {
		std::vector<std::string> desc{
			std::format("node {}; parent {}; level {}; {}",
			  getCurNodeId(), getCurParentNodeId(), getCurLevel(), label),
		};
		if (sourceRange.isValid()) {
			desc.push_back("====");
			desc.push_back(getSourceText(*sourceManager_, *langOpts_,
			  sourceRange));
			desc.push_back("====");
		}
		return desc;
	}

	template<std::ranges::range R>
	void printRange(const R& range) {
		if (logLevel_ >= 1) {
			for (const std::string& s : range) {
				llvm::outs() << s << '\n';
			}
		}
	}

	void Visit(const clang::comments::Comment *C,
	  const clang::comments::FullComment *FC) {
		++visitCount_;
		if (!C) {
			treeFormatter_.addNodeLines(makeDesc("__null__ comments::Comment"));
			return;
		}
		enableTraverse_ = true;
	}

	void Visit(const clang::Attr *attr) {
		++visitCount_;
		if (!attr) {
			treeFormatter_.addNodeLines(makeDesc("__null__ Attr"));
			return;
		}
		++attrCount_;
		std::vector<std::string> desc =
		  makeDesc(std::format("Attr {}", attr->getSpelling()));
		printRange(desc);
		treeFormatter_.addNodeLines(desc, lastChild_);
		enableTraverse_ = true;
	}

	void Visit(const clang::TemplateArgument &TA, clang::SourceRange R = {},
	  const clang::Decl *From = nullptr, clang::StringRef Label = {}) {
		++visitCount_;
		std::vector<std::string> desc = makeDesc("TemplateArgument");
		printRange(desc);
		treeFormatter_.addNodeLines(desc, lastChild_);
		// Note: Do not traverse into TemplateArgument nodes
		// as this can cause cycles.
		enableTraverse_ = false;
	}

	void Visit(const clang::Stmt *stmt) {
		++visitCount_;
		if (!stmt) {
			treeFormatter_.addNodeLines(makeDesc("__null__ Stmt"));
			return;
		}
		++stmtCount_;
		std::vector<std::string> desc =
		  makeDesc(stmt->getStmtClassName(), stmt->getSourceRange());
		printRange(desc);
		treeFormatter_.addNodeLines(desc, lastChild_);
		enableTraverse_ = true;
	}

	void Visit(const clang::Type *type) {
		++visitCount_;
		if (!type) {
			treeFormatter_.addNodeLines(makeDesc("__null__ Type"));
			return;
		}
		++typeCount_;
		std::vector<std::string> desc = makeDesc(std::format("{}Type", type->getTypeClassName()));
		printRange(desc);
		treeFormatter_.addNodeLines(desc, lastChild_);
		enableTraverse_ = true;
	}

	void Visit(clang::QualType qualType) {
		++visitCount_;
		std::vector<std::string> desc = makeDesc("QualType");
		printRange(desc);
		treeFormatter_.addNodeLines(desc, lastChild_);
		enableTraverse_ = true;
	}

	void Visit(clang::TypeLoc typeLoc) {
		++visitCount_;
		++typeLocCount_;
		std::vector<std::string> desc = makeDesc("TypeLoc");
		printRange(desc);
		treeFormatter_.addNodeLines(desc, lastChild_);
		enableTraverse_ = true;
	}

	void Visit(const clang::Decl *decl) {
		++visitCount_;
		if (!decl) {
			treeFormatter_.addNodeLines(makeDesc("__null__ Decl"));
			return;
		}
		++declCount_;
		auto [_, inserted] = declSet_.insert(decl);
		// Note: This can fail (at least due to TemplateArgument nodes).
		//assert(inserted);
		std::string label = std::string(decl->getDeclKindName()) + "Decl";
		if (auto namedDecl = llvm::dyn_cast<clang::NamedDecl>(decl)) {
			std::string name = namedDecl->getNameAsString();
			label += std::format("; name {}", name);
		}
		std::vector<std::string> desc = makeDesc(label, decl->getSourceRange());
		printRange(desc);
		treeFormatter_.addNodeLines(desc, lastChild_);
		enableTraverse_ = true;
	}

	void Visit(const clang::CXXCtorInitializer *init) {
		++visitCount_;
		std::vector<std::string> desc{makeDesc("CXXCtorInitializer",
		  init->getSourceRange())};
		printRange(desc);
		treeFormatter_.addNodeLines(desc, lastChild_);
		enableTraverse_ = true;
	}

	void Visit(const clang::OpenACCClause *clause) {
		++visitCount_;
		std::vector<std::string> desc = makeDesc("OpenACCClause");
		printRange(desc);
		treeFormatter_.addNodeLines(desc, lastChild_);
		enableTraverse_ = true;
	}

	void Visit(const clang::OMPClause *clause) {
		++visitCount_;
		std::vector<std::string> desc = makeDesc("OMPClause");
		printRange(desc);
		treeFormatter_.addNodeLines(desc, lastChild_);
		enableTraverse_ = true;
	}

	void Visit(const clang::BlockDecl::Capture& capture) {
		++visitCount_;
		std::vector<std::string> desc = makeDesc("BlockDecl::Capture");
		printRange(desc);
		treeFormatter_.addNodeLines(desc, lastChild_);
		enableTraverse_ = true;
	}

	void Visit(const clang::GenericSelectionExpr::ConstAssociation &A) {
		++visitCount_;
		std::vector<std::string> desc =
		  makeDesc("GenericSelectionExpr::ConstAssociation");
		printRange(desc);
		treeFormatter_.addNodeLines(desc, lastChild_);
		enableTraverse_ = true;
	}

	void Visit(const clang::concepts::Requirement* require) {
		++visitCount_;
		std::vector<std::string> desc = makeDesc("concepts::Requirement");
		printRange(desc);
		treeFormatter_.addNodeLines(desc, lastChild_);
		enableTraverse_ = true;
	}

	void Visit(const clang::APValue &Value, clang::QualType Ty) {
		++visitCount_;
		std::vector<std::string> desc = makeDesc("APValue");
		printRange(desc);
		treeFormatter_.addNodeLines(desc, lastChild_);
		enableTraverse_ = true;
	}

	BigInt makeNodeId() {
		return ++nextNodeId_;
	}

	void setLogLevel(int logLevel) {
		logLevel_ = logLevel;
	}

	void setOutput(llvm::raw_ostream* out) {
		out_ = out;
	}

private:

	NodeStack nodeStack_;
	bool topLevel_ = true;
	BigInt nextNodeId_ = -1;
	bool enableTraverse_;
	llvm::raw_ostream* out_;
	clang::SourceManager* sourceManager_;
	const clang::LangOptions* langOpts_;
	int logLevel_ = 0;
	TreeFormatter<llvm::raw_ostream> treeFormatter_;
	std::set<const clang::Decl*> declSet_;
	bool lastChild_;

	BigInt visitCount_ = 0;
	BigInt attrCount_ = 0;
	BigInt declCount_ = 0;
	BigInt stmtCount_ = 0;
	BigInt typeCount_ = 0;
	BigInt typeLocCount_ = 0;

	bool enableLastChild_;
};

class AstDumper : public clang::ASTNodeTraverser<AstDumper, AstNodeDumper> {
public:

	struct Stats {
		std::size_t visitCount;
		std::size_t attrCount;
		std::size_t declCount;
		std::size_t stmtCount;
		std::size_t typeCount;
		std::size_t typeLocCount;
	};

	AstDumper(clang::SourceManager& sourceManager,
	  const clang::LangOptions& langOpts, llvm::raw_ostream& out,
	  bool enableLastChild, bool flushLeft) :
	  nodeDumper_(sourceManager, langOpts, out, enableLastChild,
	  flushLeft) {}

	AstDumper(const AstDumper&) = delete;
	AstDumper& operator=(const AstDumper&) = delete;

	AstNodeDumper& doGetNodeDelegate() {
		return nodeDumper_;
	}

	void setOutput(llvm::raw_ostream* out) {
		nodeDumper_.setOutput(out);
	}

	void setLogLevel(int logLevel) {
		nodeDumper_.setLogLevel(logLevel);
	}

	void getStats(Stats& stats) {
		stats.visitCount = nodeDumper_.visitCount_;
		stats.attrCount = nodeDumper_.attrCount_;
		stats.declCount = nodeDumper_.declCount_;
		stats.stmtCount = nodeDumper_.stmtCount_;
		stats.typeCount = nodeDumper_.typeCount_;
	}

private:

	AstNodeDumper nodeDumper_;

};

#endif
