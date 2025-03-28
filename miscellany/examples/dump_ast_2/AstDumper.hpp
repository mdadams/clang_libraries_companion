#ifndef AstDumper_hpp
#define AstDumper_hpp

#define ENABLE_TYPEVISITOR
#define ENABLE_BUILTINTYPE

#include <format>
#include <stack>
#include <ranges>
#include <numeric>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/TypeLoc.h>
#include <clang/AST/Type.h>
#include <clang/AST/TypeVisitor.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTTypeTraits.h>
#include <llvm/Support/raw_ostream.h>
#include <clang/AST/PrettyPrinter.h>

#include "clang_utility_1.hpp"
#include "TreeFormatter.hpp"

class AstDumper;

class DumpTypeVisitor : public clang::TypeVisitor<DumpTypeVisitor> {
public:
	DumpTypeVisitor(AstDumper* dumper) : dumper_(dumper) {}
	AstDumper* dumper_;
	void VisitType(const clang::Type* type);
#ifdef ENABLE_BUILTINTYPE
	void VisitBuiltinType(const clang::BuiltinType* type);
#endif
};

class AstDumper : public clang::RecursiveASTVisitor<AstDumper> {
public:

	friend class DumpTypeVisitor;

	/*
	Types
	*/

	using Base = clang::RecursiveASTVisitor<AstDumper>;

	using BigInt = long long;

	struct StackEntry {
		BigInt nodeId;
		int level;
		clang::DynTypedNode node;
	};

	struct Stats {
		std::size_t totalCount;
		std::size_t attrCount;
		std::size_t declCount;
		std::size_t stmtCount;
		std::size_t typeCount;
		std::size_t typeLocCount;
		std::size_t conceptRefCount;
		std::size_t cxxBaseSpecCount;
		std::size_t ctorInitCount;
		std::size_t lambdaCaptureCount;
		std::size_t nestedNameSpecCount;
		std::size_t nestedNameSpecLocCount;
		std::size_t tempArgCount;
		std::size_t tempArgLocCount;
		std::size_t tempNameCount;
		std::size_t objcProtocolLocCount;
	};

	/*
	AST Dumper API
	*/

	AstDumper(clang::SourceManager& sourceManager,
	  const clang::LangOptions& langOpts, llvm::raw_ostream* out) :
	  sourceManager_(&sourceManager), langOpts_(&langOpts), out_(out),
	  treeFormatter_() {
		treeFormatter_.setOutput(*out);
	}

	void setPrefix(const std::string& prefix) {
		treeFormatter_.setPrefix(prefix);
	}

	void setLogLevel(int logLevel) {
		logLevel_ = logLevel;
	}

	void doTemplateInstantiations(bool flag) {
		doTemplateInstantiations_ = flag;
	}

	void getStats(Stats& stats) const {
		stats.totalCount = visitCount_;
		stats.attrCount = attrCount_;
		stats.declCount = declCount_;
		stats.stmtCount = stmtCount_;
		stats.typeCount = typeCount_;
		stats.typeLocCount = typeLocCount_;
		stats.conceptRefCount = conceptRefCount_;
		stats.cxxBaseSpecCount = cxxBaseSpecCount_;
		stats.ctorInitCount = ctorInitCount_;
		stats.lambdaCaptureCount = lambdaCaptureCount_;
		stats.nestedNameSpecCount = nestedNameSpecCount_;
		stats.nestedNameSpecLocCount = nestedNameSpecLocCount_;
		stats.tempArgCount = tempArgCount_;
		stats.tempArgLocCount = tempArgLocCount_;
		stats.tempNameCount = tempNameCount_;
		stats.objcProtocolLocCount = objcProtocolLocCount_;
	}

	/*
	Internal use only.
	*/

	int getCurLevel() const {
		if (!nodeStack_.empty()) {
			return nodeStack_.back().level;
		} else {
			return -1;
		}
	}

	BigInt getCurNodeId() const {
		assert(nodeStack_.size() >= 1);
		return nodeStack_.back().nodeId;
	}

	BigInt getCurParentNodeId() const {
		if (nodeStack_.size() >= 2) {
			return nodeStack_[nodeStack_.size() - 2].nodeId;
		} else {
			return -1;
		}
	}

	BigInt makeNodeId() {
		return ++nextNodeId_;
	}

	static std::string newlineTerm(const std::string& s) {
		return s.ends_with('\n') ? s : s + "\n";
	}

	std::string makeDesc(const std::string& label,
	  clang::SourceRange sourceRange = {}) const {
		std::string desc;
		desc += std::format("node {}; parent {}; level {}; {}\n",
			  getCurNodeId(), getCurParentNodeId(), getCurLevel(), label);
		if (sourceRange.isValid()) {
			desc += "====\n";
			desc += newlineTerm(getSourceText(*sourceManager_, *langOpts_,
			  sourceRange));
			desc += "====\n";
		}
		return desc;
	}

	template <typename... Args>
	void logf(int level, std::format_string<Args...> fmt, Args&&... args) {
		if (logLevel_ >= level) {
			*out_ << std::format(fmt, std::forward<Args>(args)...);
		}
	}

	/************************************************************\
	Some query functions.
	\************************************************************/

	bool shouldVisitImplicitCode() {
		return true;
	}

	// Note: We must use preorder traversal (not postorder).
	bool shouldTraversePostOrder() {
		// We must return false.
		return false;
	}

	bool shouldVisitTemplateInstantiations() {
		return doTemplateInstantiations_;
	}

	bool shouldWalkTypesOfTypeLocs() {
		// NOTE: If true is returned, multiple visits will be performed
		// for TypeLoc nodes, which will mess up the formatted tree output.
		return true;
	}

	bool shouldVisitLambdaBody() {
		return true;
	}

	/************************************************************\
	Traversal Functions
	\************************************************************/

	template<class Node, class TraverseNode, class VisitNode>
	bool traverseImpl1a(Node* node, TraverseNode traverseNode,
	  VisitNode visitNode, const std::string label = {}) {
		if (!node) {
			return true;
		}
		BigInt nodeId = makeNodeId();
		logf(1, "TraverseImpl entered {} {}\n", nodeId, label);
		// Note: The DynTypedNode is not really used for anything currently.
		nodeStack_.emplace_back(nodeId, getCurLevel() + 1,
		  clang::DynTypedNode::create(*node));
		treeFormatter_.down();
		bool ret = visitNode(node);
		if (ret) {
			ret = traverseNode(node);
		}
		treeFormatter_.up();
		nodeStack_.pop_back();
		logf(1, "TraverseImpl exited {} {}\n", nodeId, label);
		return ret;
	}

	template<class Node, class IsNull, class TraverseNode, class VisitNode>
	bool traverseImpl3a(Node node, IsNull isNull, TraverseNode traverseNode,
	  VisitNode visitNode, const std::string& label = {}) {
		if (isNull(node)) {
			return true;
		}
		BigInt nodeId = makeNodeId();
		logf(1, "TraverseImpl entered {} {}\n", nodeId, label);
		// Note: The DynTypedNode is not really used for anything currently.
		nodeStack_.emplace_back(nodeId, getCurLevel() + 1,
		  clang::DynTypedNode::create(node));
		treeFormatter_.down();
		bool ret = visitNode(node);
		if (ret) {
			ret = traverseNode(node);
		}
		treeFormatter_.up();
		nodeStack_.pop_back();
		logf(1, "TraverseImpl exited {} {}\n", nodeId, label);
		return ret;
	}

	//////////

	/*
	The following are not currently handled:
	TraverseSynOrSemInitListExpr
	TraverseTemplateInstantiations (overloaded)
	TraverseConceptRequirement
	*/

	bool TraverseAttr(clang::Attr* attr) {
		return traverseImpl1a(attr,
		  [this](auto attr){return Base::TraverseAttr(attr);},
		  [this](auto attr){return xVisitAttr(attr);},
		  "Attr");
	}

	bool TraverseConceptReference(clang::ConceptReference* cr) {
		return traverseImpl1a(cr,
		  [this](auto cr){return Base::TraverseConceptReference(cr);},
		  [this](auto cr){return xVisitConceptReference(cr);},
		  "ConceptReference");
	}

	bool TraverseCXXBaseSpecifier(const clang::CXXBaseSpecifier& spec) {
		return traverseImpl3a(spec,
		  [this](auto spec){return false;},
		  [this](auto spec){return Base::TraverseCXXBaseSpecifier(spec);},
		  [this](auto spec){return xVisitCXXBaseSpecifier(spec);},
		  "CXXBaseSpecifier");
	}

	bool TraverseConstructorInitializer(clang::CXXCtorInitializer* node) {
		return traverseImpl1a(node,
		  [this](auto node){return Base::TraverseConstructorInitializer(node);},
		  [this](auto node){return xVisitConstructorInitializer(node);},
		  "CXXCtorInitializer");
	}

	bool TraverseDecl(clang::Decl* decl) {
		return traverseImpl1a(decl,
		  [this](auto decl){return Base::TraverseDecl(decl);},
		  [this](auto decl){return xVisitDecl(decl);},
		  "Decl");
	}

	bool TraverseLambdaCapture(clang::LambdaExpr *lambda,
	  const clang::LambdaCapture *capture, clang::Expr *init) {
		xVisitLambdaCapture(capture);
		if (lambda->isInitCapture(capture)) {
			if (!TraverseDecl(capture->getCapturedVar())) {
				return false;
			}
		} else {
			if (!TraverseStmt(init)) {
				return false;
			}
		}
		return true;
	}

	bool TraverseNestedNameSpecifier(clang::NestedNameSpecifier* nns) {
		return traverseImpl1a(nns,
		  [this](auto nns){return Base::TraverseNestedNameSpecifier(nns);},
		  [this](auto nns){return xVisitNestedNameSpecifier(nns);},
		  "NestedNameSpecifier");
	}

	bool TraverseNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc loc) {
		return traverseImpl3a(loc, [this](auto loc){return !loc;},
		  [this](auto loc){return Base::TraverseNestedNameSpecifierLoc(loc);},
		  [this](auto loc){return xVisitNestedNameSpecifierLoc(loc);},
		  "NestedNameSpecifierLoc");
	}

	bool TraverseObjCProtocolLoc(clang::ObjCProtocolLoc loc) {
		// NOTE/FIXME: This should probably actually do something.
		// Effectively, ignore ObjCProcotolLoc nodes.
		++objcProtocolLocCount_;
		return true;
	}

#if 0
	// NOTE: This will not compile since Base::TraverseOMPClause is private.
	bool TraverseOMPClause(clang::OMPClause* node) {
		return traverseImpl1a(node,
		  [this](auto node){return Base::TraverseOMPClause(node);},
		  [this](auto node){return xVisitOMPClause(node);},
		  "OMPClause");
	}
#endif

	bool TraverseStmt(clang::Stmt* stmt) {
		return traverseImpl1a(stmt,
		  [this](auto stmt){return Base::TraverseStmt(stmt);},
		  [this](auto stmt){return xVisitStmt(stmt);},
		  "Stmt");
	}

	bool TraverseTemplateArgument(clang::TemplateArgument arg) {
		return traverseImpl3a(arg,
		  [this](auto arg){return false;},
		  [this](auto arg){return Base::TraverseTemplateArgument(arg);},
		  [this](auto arg){return xVisitTemplateArgument(arg);},
		  "TemplateArgument");
	}

	bool TraverseTemplateArgumentLoc(clang::TemplateArgumentLoc loc) {
		return traverseImpl3a(loc,
		  [this](auto loc){return false;},
		  [this](auto loc){return Base::TraverseTemplateArgumentLoc(loc);},
		  [this](auto loc){return xVisitTemplateArgumentLoc(loc);},
		  "TemplateArgumentLoc");
	}

	bool TraverseTemplateName(clang::TemplateName name) {
		return traverseImpl3a(name,
		  [this](auto name){return false;},
		  [this](auto name){return Base::TraverseTemplateName(name);},
		  [this](auto name){return xVisitTemplateName(name);},
		  "TemplateName");
	}

	bool TraverseType(clang::QualType qualType) {
		return traverseImpl3a(qualType,
		  [this](auto qualType){return qualType.isNull();},
		  [this](auto qualType){return Base::TraverseType(qualType);},
		  [this](auto qualType){return xVisitType(qualType.getTypePtr());},
		  "QualType");
	}

	bool TraverseTypeLoc(clang::TypeLoc typeLoc) {
		return traverseImpl3a(typeLoc,
		  [this](auto typeLoc){return typeLoc.isNull();},
		  [this](auto typeLoc){return Base::TraverseTypeLoc(typeLoc);},
		  [this](auto typeLoc){return xVisitTypeLoc(typeLoc);},
		  "TypeLoc");
	}

	/************************************************************\
	Visit Functions
	\************************************************************/

	bool xVisitAttr(clang::Attr* attr) {
		++visitCount_;
		++attrCount_;
		assert(attr);
		std::string desc{makeDesc(
		  std::format("Attr {}", attr->getSpelling()))};
		logf(1, "{}", desc);
		treeFormatter_.addNode(desc);
		return true;
	}

	bool xVisitConceptReference(clang::ConceptReference* cr) {
		++visitCount_;
		++conceptRefCount_;
		treeFormatter_.addNode(makeDesc("ConceptReference"));
		return true;
	}

	bool xVisitCXXBaseSpecifier(clang::CXXBaseSpecifier loc) {
		++visitCount_;
		++cxxBaseSpecCount_;
		treeFormatter_.addNode(makeDesc("CXXBaseSpecifier"));
		return true;
	}

	bool xVisitConstructorInitializer(clang::CXXCtorInitializer* init) {
		++visitCount_;
		++ctorInitCount_;
		treeFormatter_.addNode(makeDesc("CXXCtorInitializer"));
		return true;
	}

	bool xVisitDecl(clang::Decl* decl) {
		++visitCount_;
		++declCount_;
		assert(decl);
		std::string label = std::format("{}Decl", decl->getDeclKindName());
		if (auto namedDecl = llvm::dyn_cast<clang::NamedDecl>(decl);
		  namedDecl) {
			label += std::format("; name {}", namedDecl->getNameAsString());
		}
		std::string desc = makeDesc(label, decl->getSourceRange());
		logf(1, "{}", desc);
		treeFormatter_.addNode(desc);
		return true;
	}

	// ???
	bool xVisitLambdaCapture(const clang::LambdaCapture *capture) {
		++visitCount_;
		++lambdaCaptureCount_;
		std::string desc = makeDesc("LambdaCapture");
		logf(1, "{}", desc);
		treeFormatter_.addNode(desc);
		return true;
	}

	bool xVisitNestedNameSpecifier(clang::NestedNameSpecifier* nns) {
		++visitCount_;
		++nestedNameSpecCount_;
		std::string desc = makeDesc("NestedNameSpecifier");
		logf(1, "{}", desc);
		treeFormatter_.addNode(desc);
		return true;
	}

	bool xVisitNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc loc) {
		++visitCount_;
		++nestedNameSpecLocCount_;
		std::string desc = makeDesc("NestedNameSpecifierLoc");
		logf(1, "{}", desc);
		treeFormatter_.addNode(desc);
		return true;
	}

#if 0
	// NOTE: This is not currently used.
	bool xVisitObjCProtocolLoc(clang::ObjCProtocolLoc loc) {
		return true;
	}
#endif

	bool xVisitOMPClause(clang::OMPClause* node) {
		++visitCount_;
		assert(node);
		return true;
	}

	bool xVisitStmt(clang::Stmt* stmt) {
		++visitCount_;
		++stmtCount_;
		assert(stmt);
		std::string desc = makeDesc(std::format("type {}",
		  stmt->getStmtClassName()), stmt->getSourceRange());
		logf(1, "{}", desc);
		treeFormatter_.addNode(desc);
		return true;
	}

	bool xVisitTemplateArgument(clang::TemplateArgument arg) {
		++visitCount_;
		++tempArgCount_;
		std::string desc = makeDesc("TemplateArgument");
		logf(1, "{}", desc);
		treeFormatter_.addNode(desc);
		return true;
	}

	bool xVisitTemplateArgumentLoc(clang::TemplateArgumentLoc loc) {
		++visitCount_;
		++tempArgLocCount_;
		std::string desc = makeDesc("TemplateArgumentLoc");
		logf(1, "{}", desc);
		treeFormatter_.addNode(desc);
		return true;
	}

	bool xVisitTemplateName(clang::TemplateName name) {
		++visitCount_;
		++tempNameCount_;
		std::string desc = makeDesc("TemplateName");
		logf(1, "{}", desc);
		treeFormatter_.addNode(desc);
		return true;
	}

#ifndef ENABLE_TYPEVISITOR

	bool VisitType(clang::Type* type) {
		++visitCount_;
		++typeCount_;
		assert(type);
		std::string name = type->getTypeClassName();
		std::string desc = makeDesc(std::format("type {}Type",
		  type->getTypeClassName()));
		logf(1, "{}", desc);
		treeFormatter_.addNode(desc);
		return true;
	}

#else

	bool xVisitType(const clang::Type* type) {
		DumpTypeVisitor visitor(this);
		visitor.Visit(type);
		++visitCount_;
		++typeCount_;
		assert(type);
		return true;
	}

#endif

	bool xVisitTypeLoc(clang::TypeLoc typeLoc) {
		++visitCount_;
		++typeLocCount_;
		assert(!typeLoc.isNull());
		std::string desc = makeDesc(std::format("type {}TypeLoc",
		  typeLoc.getType().getTypePtr()->getTypeClassName()),
		  typeLoc.getSourceRange());
		logf(1, "{}", desc);
		treeFormatter_.addNode(desc);
		return true;
	}

private:

	/************************************************************\
	State
	\************************************************************/

	std::deque<StackEntry> nodeStack_;
	llvm::raw_ostream* out_;
	int logLevel_ = 0;
	BigInt nextNodeId_ = -1;
	clang::SourceManager* sourceManager_;
	const clang::LangOptions* langOpts_;
	TreeFormatter<llvm::raw_ostream> treeFormatter_;
	BigInt visitCount_ = 0;
	BigInt attrCount_ = 0;
	BigInt declCount_ = 0;
	BigInt stmtCount_ = 0;
	BigInt typeCount_ = 0;
	BigInt typeLocCount_ = 0;
	std::size_t conceptRefCount_ = 0;
	std::size_t cxxBaseSpecCount_ = 0;
	std::size_t ctorInitCount_ = 0;
	std::size_t lambdaCaptureCount_ = 0;
	std::size_t nestedNameSpecCount_ = 0;
	std::size_t nestedNameSpecLocCount_ = 0;
	std::size_t tempArgCount_ = 0;
	std::size_t tempArgLocCount_ = 0;
	std::size_t tempNameCount_ = 0;
	std::size_t objcProtocolLocCount_ = 0;

	bool doTemplateInstantiations_;
};

inline void DumpTypeVisitor::VisitType(const clang::Type* type)
{
	TreeFormatter<llvm::raw_ostream>& treeFormatter_ = dumper_->treeFormatter_;
	std::string name = type->getTypeClassName();
	std::string desc = dumper_->makeDesc(std::format("type {}Type",
	  type->getTypeClassName()));
	dumper_->logf(1, "{}", desc);
	dumper_->treeFormatter_.addNode(desc);
}

#ifdef ENABLE_BUILTINTYPE
inline void DumpTypeVisitor::VisitBuiltinType(const clang::BuiltinType* type)
{
	clang::PrintingPolicy printPolicy(*dumper_->langOpts_);
	TreeFormatter<llvm::raw_ostream>& treeFormatter_ = dumper_->treeFormatter_;
	std::string name = type->getTypeClassName();
	std::string desc = dumper_->makeDesc(std::format("type {}Type {}",
	  type->getTypeClassName(), std::string(type->getName(printPolicy))));
	dumper_->logf(1, "{}", desc);
	dumper_->treeFormatter_.addNode(desc);
}
#endif

#endif
