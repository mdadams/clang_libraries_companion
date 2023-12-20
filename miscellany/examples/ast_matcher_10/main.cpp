#include <format>
#include <llvm/Support/raw_ostream.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <cal/main.hpp>

#include "clang_utility.hpp"
#ifdef ENABLE_EXPERIMENTAL
#include "clang_experimental.hpp"
#endif

namespace ct = clang::tooling;
namespace cam = clang::ast_matchers;
using namespace std::literals;

static llvm::cl::OptionCategory optionCategory("Tool options");
static int clVerbose = 0;
static llvm::cl::opt<bool> clDummyVerbose("v", llvm::cl::cat(optionCategory),
  llvm::cl::callback([](const bool& value){++clVerbose;}));
static llvm::cl::opt<std::string> clClangIncludeDir(
  "I", llvm::cl::desc("Clang include directory"),
  llvm::cl::cat(optionCategory));
static llvm::cl::opt<int> clDeclMatcherId(
  "d", llvm::cl::desc("Matcher ID"), llvm::cl::value_desc("matcher_id"),
  llvm::cl::cat(optionCategory), llvm::cl::init(-1));
static llvm::cl::opt<int> clStmtMatcherId(
  "s", llvm::cl::desc("Matcher ID"), llvm::cl::value_desc("matcher_id"),
  llvm::cl::cat(optionCategory), llvm::cl::init(-1));
static llvm::cl::opt<bool> clIgnoreImplicit(
  "ignore-implicit", llvm::cl::desc("Ignore implicit nodes"),
  llvm::cl::cat(optionCategory), llvm::cl::init(false));
static llvm::cl::opt<bool> clDumpAst(
  "dump-ast", llvm::cl::desc("Dump AST for match"),
  llvm::cl::cat(optionCategory), llvm::cl::init(false));

unsigned int getDepth(clang::ASTContext& astContext,
  const clang::DynTypedNode* node) {
	unsigned int count = 0;
	const clang::DynTypedNode* curNode = node;
	for (;;) {
		auto parents = astContext.getParents(*curNode);
		if (parents.size() == 0) {
			break;
		}
		if (parents.size() > 1) {
			llvm::outs() << std::format("multiple parents {}\n",
			  parents.size());
		}
		++count;
		curNode = &parents[0];
	}
	return count;
}

clang::DynTypedNode getFarAncestor(clang::ASTContext& astContext,
  const clang::DynTypedNode* node) {
	const clang::DynTypedNode* curNode = node;
	clang::DynTypedNode parentNode;
	for (;;) {
		auto parents = astContext.getParents(*curNode);
		if (parents.size() == 0) {
			break;
		}
		if (parents.size() > 1) {
			llvm::outs() << std::format("multiple parents {}\n",
			  parents.size());
		}
		curNode = &parents[0];
		parentNode = *curNode;
	}
	return parentNode;
}

clang::DynTypedNode getParent(clang::ASTContext& astContext,
  const clang::DynTypedNode* node) {
	auto parents = astContext.getParents(*node);
	clang::DynTypedNode parentNode;
	if (parents.size() > 0) {
		if (parents.size() > 1) {
			llvm::outs() << std::format("multiple parents {}\n",
			  parents.size());
		}
		parentNode = parents[0];
	} else {
		parentNode = clang::DynTypedNode();
	}
	return parentNode;
}

clang::SourceRange charSourceRangeToSourceRange(const clang::SourceManager&
  sourceManager, clang::CharSourceRange charSourceRange) {
	return clang::SourceRange(
	  getBeginningOfToken(sourceManager, charSourceRange.getBegin()),
	  getBeginningOfToken(sourceManager, charSourceRange.getEnd()));
}

AST_MATCHER(clang::Decl, hasComment) {
	if (auto p = Finder->getASTContext().getCommentForDecl(&Node, nullptr)) {
		return true;
	} else {
		return false;
	}
}

AST_MATCHER(clang::CXXMethodDecl, isSpecialMember) {
	bool result;
	if (auto p = llvm::dyn_cast<clang::CXXConstructorDecl>(&Node)) {
		result = p->isDefaultConstructor() || p->isCopyConstructor() ||
		  p->isMoveConstructor();
	} else if (auto p = llvm::dyn_cast<clang::CXXDestructorDecl>(&Node)) {
		result = true;
	} else {
		result = Node.isCopyAssignmentOperator() ||
		  Node.isMoveAssignmentOperator();
	}
	return result;
}

AST_MATCHER_P(clang::CXXMethodDecl, paramCountAtLeast, unsigned, Threshold) {
	return Node.param_size() >= Threshold;
}

AST_MATCHER_P(clang::CXXMethodDecl, hasNumOverrides, unsigned, N) {
	const auto& node = Node;
	return node.size_overridden_methods() >= N;
}

AST_MATCHER_P(clang::NamedDecl, nameLengthAtLeast, unsigned, Threshold) {
	return Node.getIdentifier() && Node.getName().size() >= Threshold;
}

cam::DeclarationMatcher getDeclMatcher(int id) {
	using namespace cam;
	switch (id) {
	default:
	case 0:
		return decl().bind("x");
	case 1:
		return namedDecl().bind("x");
	case 2:
		return varDecl().bind("x");
	case 3:
		return functionDecl().bind("x");
	case 4:
		return cxxMethodDecl().bind("x");
	case 5:
		return recordDecl().bind("x");
	case 6:
		return cxxRecordDecl().bind("x");
	// gap in numbering of cases
	case 40:
		return decl(hasComment()).bind("x");
	// gap in numbering of cases
	case 50:
		return cxxMethodDecl(isDefinition(), isSpecialMember(),
		  unless(isImplicit())).bind("x");
	case 51:
		return cxxMethodDecl(paramCountAtLeast(4)).bind("x");
	case 52:
		return cxxMethodDecl(hasNumOverrides(1)).bind("x");
	case 53:
		return namedDecl(nameLengthAtLeast(6)).bind("x");
	}
}

cam::StatementMatcher getStmtMatcher(int id) {
	using namespace cam;
	switch (id) {
	default:
	case 0:
		return stmt().bind("x");
	case 1:
		return expr().bind("x");
	case 2:
		return callExpr().bind("x");
	case 3:
		return ifStmt().bind("x");
	case 4:
		return switchStmt().bind("x");
	case 5:
		return forStmt().bind("x");
	case 6:
		return whileStmt().bind("x");
	case 7:
		return doStmt().bind("x");
	case 8:
		return materializeTemporaryExpr().bind("x");
	}
}

bool printMatch(clang::SourceManager& sourceManager, clang::SourceRange
  sourceRange) {
	bool status = true;

	assert(sourceRange.isValid());

	clang::CharSourceRange expRange = sourceManager.getExpansionRange(
	  sourceRange);
	clang::SourceRange expTokenRange = charSourceRangeToSourceRange(
	  sourceManager, expRange);
	auto expFileName = std::string(sourceManager.getFilename(
	  sourceManager.getExpansionLoc(expRange.getBegin())));
	unsigned expBeginLineNum = sourceManager.getExpansionLineNumber(
	  expRange.getBegin());
	unsigned expBeginColumnNum = sourceManager.getExpansionColumnNumber(
	  expRange.getBegin());
	unsigned expEndLineNum = sourceManager.getExpansionLineNumber(
	  expRange.getEnd());
	unsigned expEndColumnNum = sourceManager.getExpansionColumnNumber(
	  expRange.getEnd());
	auto expEndFileName = std::string(sourceManager.getFilename(
	  sourceManager.getExpansionLoc(expRange.getEnd())));

	auto [validText, text] = charSourceRangeToText(sourceManager, expRange);
	if (!validText) {
		status = false;
	}
	llvm::outs()
	  << std::format("expansion range {}:{}({})-{}:{}({})\n", expFileName,
	  expBeginLineNum, expBeginColumnNum, expEndFileName, expEndLineNum,
	  expEndColumnNum)
	  << std::format("\nexpansion range text:\n{}\n",
	  validText ?  cal::addLineNumbers(text, expBeginLineNum,
	  expBeginColumnNum, true, true) : "[invalid]\n");

	llvm::outs()
	  << std::format("spelling location {}:{}({})\n",
	  std::string(sourceManager.getFilename(sourceManager.getSpellingLoc(
	  sourceRange.getBegin()))),
	  sourceManager.getSpellingLineNumber(sourceRange.getBegin()),
	  sourceManager.getSpellingColumnNumber(sourceRange.getBegin()));

#if 0
	clang::SourceLocation spellRangeBegin = sourceManager.getSpellingLoc(
	  sourceRange.getBegin());
	clang::SourceLocation spellRangeEnd = sourceManager.getSpellingLoc(
	  sourceRange.getEnd());
	unsigned spellBeginLineNum = sourceManager.getSpellingLineNumber(
      spellRangeBegin);
	unsigned spellBeginColumnNum = sourceManager.getSpellingColumnNumber(
      spellRangeBegin);
	clang::SourceRange spellRange(spellRangeBegin, spellRangeEnd);
	llvm::outs()
	  << std::format("\nspelling range text:\n{}\n",
	  cal::addLineNumbers(sourceRangeToText(sourceManager, spellRange, true).second,
	  spellBeginLineNum, spellBeginColumnNum, true, true));
#endif

	if (expTokenRange != sourceRange) {
		auto [valid, text] = sourceRangeToText(sourceManager, sourceRange);
		if (valid) {
			llvm::outs() << std::format("\nsource range:\n{}\n",
			  cal::addLineNumbers(text, 1, 1, true, true));
		} else {
			llvm::outs() <<
			  "cannot print range (probably in macro expansion)\n";
		}
	} else {
		llvm::outs() << "source range same as expansion range\n";
	}

	llvm::outs()
	  << std::format("expansion is token range: {}\n",
	  expRange.isTokenRange())
	  << std::format("sourceRange.getBegin().isMacroID(): {}\n",
	  sourceRange.getBegin().isMacroID());

#ifdef ENABLE_EXPERIMENTAL
	examineSourceLocation(llvm::outs(), sourceManager, sourceRange.getBegin());
	examineSourceLocation(llvm::outs(), sourceManager, sourceRange.getEnd());
#endif
	return status;
}

class MyMatchCallback : public cam::MatchFinder::MatchCallback {
public:
	MyMatchCallback() : count_(0) {}
	void run(const cam::MatchFinder::MatchResult& result) override {
		clang::ASTContext& astContext = *result.Context;
		clang::SourceManager& sourceManager = astContext.getSourceManager();
		clang::SourceRange sourceRange;
		clang::SourceRange altSourceRange;
		clang::SourceLocation sourceLocation;
		std::string nodeType;
		std::string name;
		std::string dumpOutput;
		llvm::raw_string_ostream dumpStream(dumpOutput);
		clang::DynTypedNode node;

		bool found = false;
		if (auto p = result.Nodes.getNodeAs<clang::Stmt>("x")) {
			found = true;
			if (auto p = result.Nodes.getNodeAs<clang::CallExpr>("x")) {
				nodeType = "CallExpr";
				sourceRange = p->getSourceRange();
				assert(sourceRange.getBegin() == p->getBeginLoc());
				assert(sourceRange.getEnd() == p->getEndLoc());
				// name not set
				p->dump(dumpStream, astContext);
				//parents = astContext.getParents(*p);
				node = clang::DynTypedNode::create(*p);
			} else if (auto p = result.Nodes.getNodeAs<clang::IfStmt>("x")) {
				nodeType = "IfStmt";
				sourceRange = p->getSourceRange();
				// name not set
				p->dump(dumpStream, astContext);
				node = clang::DynTypedNode::create(*p);
			} else if (auto p =
			  result.Nodes.getNodeAs<clang::CompoundStmt>("x")) {
				nodeType = "CompoundStmt";
				sourceRange = p->getSourceRange();
				// name not set
				p->dump(dumpStream, astContext);
				node = clang::DynTypedNode::create(*p);
			} else if (auto p = result.Nodes.getNodeAs<clang::Expr>("x")) {
				nodeType = "Expr";
				sourceRange = p->getSourceRange();
				// name not set
				p->dump(dumpStream, astContext);
				node = clang::DynTypedNode::create(*p);
			} else if (auto p = result.Nodes.getNodeAs<clang::Stmt>("x")) {
				nodeType = "Stmt";
				sourceRange = p->getSourceRange();
				// name not set
				p->dump(dumpStream, astContext);
				node = clang::DynTypedNode::create(*p);
			} else {
				found = false;
			}
		} else if (result.Nodes.getNodeAs<clang::Decl>("x")) {
			found = true;
			if (auto p = result.Nodes.getNodeAs<clang::CXXMethodDecl>("x")) {
				nodeType = "CXXMethodDecl";
				sourceRange = p->getSourceRange();
				assert(sourceRange.getBegin() == p->getBeginLoc());
				sourceLocation = p->getLocation();
				name = p->getQualifiedNameAsString();
				p->dump(dumpStream);
				node = clang::DynTypedNode::create(*p);
			} else if (auto p =
			  result.Nodes.getNodeAs<clang::FunctionDecl>("x")) {
				nodeType = "FunctionDecl";
				sourceRange = p->getSourceRange();
				assert(sourceRange.getBegin() == p->getBeginLoc());
				sourceLocation = p->getLocation();
				name = p->getQualifiedNameAsString();
				p->dump(dumpStream);
				node = clang::DynTypedNode::create(*p);
			} else if (auto p =
			  result.Nodes.getNodeAs<clang::ParmVarDecl>("x")) {
				nodeType = "ParmVarDecl";
				sourceRange = p->getSourceRange();
				assert(sourceRange.getBegin() == p->getBeginLoc());
				sourceLocation = p->getLocation();
				name = p->getQualifiedNameAsString();
				p->dump(dumpStream);
				node = clang::DynTypedNode::create(*p);
			} else if (auto p = result.Nodes.getNodeAs<clang::VarDecl>("x")) {
				nodeType = "VarDecl";
				sourceRange = p->getSourceRange();
				assert(sourceRange.getBegin() == p->getBeginLoc());
				sourceLocation = p->getLocation();
				name = p->getQualifiedNameAsString();
				p->dump(dumpStream);
				node = clang::DynTypedNode::create(*p);
			} else if (auto p =
			  result.Nodes.getNodeAs<clang::CXXRecordDecl>("x")) {
				nodeType = "CXXRecordDecl";
				sourceRange = p->getSourceRange();
				// TODO/NOTE: Why can the following assertion fail?
				// assert(sourceRange.getBegin() == p->getBeginLoc());
				sourceLocation = p->getLocation();
				name = p->getQualifiedNameAsString();
				p->dump(dumpStream);
				node = clang::DynTypedNode::create(*p);
			} else if (auto p =
			  result.Nodes.getNodeAs<clang::RecordDecl>("x")) {
				nodeType = "RecordDecl";
				sourceRange = p->getSourceRange();
				assert(sourceRange.getBegin() == p->getBeginLoc());
				sourceLocation = p->getLocation();
				name = p->getQualifiedNameAsString();
				p->dump(dumpStream);
				node = clang::DynTypedNode::create(*p);
			} else if (auto p = result.Nodes.getNodeAs<clang::NamedDecl>("x")) {
				nodeType = "NamedDecl";
				sourceRange = p->getSourceRange();
				assert(sourceRange.getBegin() == p->getBeginLoc());
				sourceLocation = p->getLocation();
				name = p->getQualifiedNameAsString();
				p->dump(dumpStream);
				node = clang::DynTypedNode::create(*p);
			} else if (auto p = result.Nodes.getNodeAs<clang::EmptyDecl>("x")) {
				nodeType = "EmptyDecl";
				sourceRange = p->getSourceRange();
				assert(sourceRange.getBegin() == p->getBeginLoc());
				sourceLocation = p->getLocation();
				p->dump(dumpStream);
				node = clang::DynTypedNode::create(*p);
				// name not set
			} else if (auto p = result.Nodes.getNodeAs<clang::Decl>("x")) {
				nodeType = "Decl";
				sourceRange = p->getSourceRange();
				assert(sourceRange.getBegin() == p->getBeginLoc());
				sourceLocation = p->getLocation();
				// name not set
				p->dump(dumpStream);
				node = clang::DynTypedNode::create(*p);
			} else {
				found = false;
			}
		}
		assert(found);
		llvm::outs()
		  << std::format("{}\nMATCH #{}\n", std::string(80, '-'), count_)
		  << std::format("type: {}\n", nodeType)
		  << std::format("name: {}\n", name);

		if (clVerbose >= 2) {
			auto parents = astContext.getParents(node);
			clang::DynTypedNode farthestAncestor =
			  getFarAncestor(astContext, &node);
			llvm::outs() << std::format("depth: {}\n",
			  getDepth(astContext, &node));
			llvm::outs() << std::format("number of parents: {}\n",
			  parents.size());
			farthestAncestor.dump(llvm::outs(), astContext);
			node.dump(llvm::outs(), astContext);
			{
				clang::DynTypedNode curNode = node;
				for (;;) {
					clang::DynTypedNode parentNode =
					  getParent(astContext, &curNode);
					llvm::outs()
					  << std::format("{}\n", std::string(80, '-'), count_);
					llvm::outs()
					  << std::format("node kind {}\n",
					  std::string(parentNode.getNodeKind().asStringRef()));
					parentNode.dump(llvm::outs(), astContext);
					curNode = parentNode;
					if (parentNode.getNodeKind().isNone()) {
						break;
					}
				}
			}
		}

		bool status = true;
		if (sourceRange.isValid()) {
			llvm::outs()
			  << std::format("begin spelling location {}:{}({})\n",
			  std::string(sourceManager.getFilename(
			  sourceManager.getSpellingLoc(sourceRange.getBegin()))),
			  sourceManager.getSpellingLineNumber(sourceRange.getBegin()),
			  sourceManager.getSpellingColumnNumber(sourceRange.getBegin()));
			llvm::outs()
			  << std::format("end spelling location {}:{}({})\n",
			  std::string(sourceManager.getFilename(
			  sourceManager.getSpellingLoc(sourceRange.getEnd()))),
			  sourceManager.getSpellingLineNumber(sourceRange.getEnd()),
			  sourceManager.getSpellingColumnNumber(sourceRange.getEnd()));
			status = printMatch(sourceManager, sourceRange);
		} else {
			llvm::outs() << "source range not valid\n";
		}
		if (sourceLocation.isValid()) {
			llvm::outs()
			  << std::format("spelling location {}:{}({})\n",
			  std::string(sourceManager.getFilename(
			  sourceManager.getSpellingLoc(sourceLocation))),
			  sourceManager.getSpellingLineNumber(sourceLocation),
			  sourceManager.getSpellingColumnNumber(sourceLocation));
		} else {
			llvm::outs() << "source location not valid\n";
		}
		if (clDumpAst || !status) {
			llvm::outs() << dumpOutput;
		}
		++count_;
	}
	unsigned getNumMatches() const {
		return count_;
	}
private:
	unsigned count_;
};

int main(int argc, const char **argv) {
	clClangIncludeDir = cal::getClangIncludeDirPathName();
	auto expectedParser = ct::CommonOptionsParser::create(argc, argv,
	  optionCategory);
	if (!expectedParser) {
		llvm::errs() << llvm::toString(expectedParser.takeError());
		return 1;
	}
	ct::CommonOptionsParser& optionsParser = expectedParser.get();
	ct::ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());
	if (!clClangIncludeDir.empty()) {
		if (clVerbose >= 1) {
			llvm::outs() << std::format("Clang include directory: {}\n",
			  std::string(clClangIncludeDir));
		}
		tool.appendArgumentsAdjuster(ct::getInsertArgumentAdjuster(("-I"s +=
		  clClangIncludeDir).c_str(), ct::ArgumentInsertPosition::BEGIN));
	}
	cam::MatchFinder matchFinder;
	MyMatchCallback matchCallback;
	if (clDeclMatcherId >= 0) {
		llvm::outs() << std::format("decl matcher {}\n",
		  static_cast<int>(clDeclMatcherId));
		cam::DeclarationMatcher matcher = getDeclMatcher(clDeclMatcherId);
		if (clIgnoreImplicit) {
			llvm::outs() << "NOTE: IGNORING IMPLICIT NODES\n";
			matcher = clang::ast_matchers::traverse(
			  clang::TK_IgnoreUnlessSpelledInSource, matcher);
		}
		matchFinder.addMatcher(matcher, &matchCallback);
	}
	if (clStmtMatcherId >= 0) {
		llvm::outs() << std::format("stmt matcher\n",
		  static_cast<int>(clStmtMatcherId));
		cam::StatementMatcher matcher = getStmtMatcher(clStmtMatcherId);
		if (clIgnoreImplicit) {
			llvm::outs() << "NOTE: IGNORING IMPLICIT NODES\n";
			matcher = clang::ast_matchers::traverse(
			  clang::TK_IgnoreUnlessSpelledInSource, matcher);
		}
		matchFinder.addMatcher(matcher, &matchCallback);
	}
	int status = tool.run(ct::newFrontendActionFactory(&matchFinder).get());
	llvm::outs() << std::format("number of matches: {}\n",
	  matchCallback.getNumMatches());
}
