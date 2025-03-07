#ifndef AstNodeCounterVisitor_hpp
#define AstNodeCounterVisitor_hpp

#include <clang/AST/RecursiveASTVisitor.h>
#include <iostream>

class AstNodeCounterVisitor :
  public clang::RecursiveASTVisitor<AstNodeCounterVisitor> {
public:
	using CountType = unsigned long long;

    explicit AstNodeCounterVisitor() {}

	bool shouldVisitImplicitCode() {
		return true;
	}

	bool shouldWalkTypesOfTypeLocs() {
		return false;
	}

	bool shouldVisitLambdaBody() {
		return true;
	}

    bool VisitAttr(clang::Attr *) {
		++attrCount;
        ++totalCount;
        return true;
    }

    bool VisitDecl(clang::Decl *) {
        ++totalCount;
		++declCount;
        return true; // Continue traversal
    }

    bool VisitStmt(clang::Stmt *) {
        ++totalCount;
		++stmtCount;
        return true;
    }

    bool VisitType(clang::Type *) {
		++typeCount;
        ++totalCount;
        return true;
    }

    bool VisitTypeLoc(clang::TypeLoc) {
		++typeLocCount;
        ++totalCount;
        return true;
    }

    CountType totalCount = 0;
    CountType attrCount = 0;
	CountType declCount = 0;
	CountType stmtCount = 0;
	CountType typeCount = 0;
	CountType typeLocCount = 0;

};

#endif
