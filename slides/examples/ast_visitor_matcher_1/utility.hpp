#include <cassert>
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ParentMapContext.h"

template<class NodeType>
const NodeType* getParentOfStmt(clang::ASTContext& astContext,
  const clang::Stmt* stmt) {
	auto parents = astContext.getParents(*stmt);
	const clang::Stmt* curStmt = nullptr;
	const NodeType* parent = nullptr;
	for (auto&& node : parents) {
		if (auto p = node.get<NodeType>()) {
			assert(!parent);
			parent = p;
		}
	}
	return parent;
}

inline unsigned getForDepth(clang::ASTContext& astContext,
  const clang::Stmt* forStmt) {
	assert(llvm::isa<clang::ForStmt>(forStmt) ||
	  llvm::isa<clang::CXXForRangeStmt>(forStmt));
	unsigned count = 1;
	const clang::Stmt* curStmt = forStmt;
	while ((curStmt = getParentOfStmt<clang::Stmt>(astContext, curStmt))) {
		if (llvm::isa<clang::ForStmt>(curStmt) ||
		  llvm::isa<clang::CXXForRangeStmt>(curStmt)) {++count;}
	}
	return count;
}
