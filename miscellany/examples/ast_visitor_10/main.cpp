#include <format>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Config/llvm-config.h>
#include <llvm/Support/CommandLine.h>
#include "utilities.hpp"

namespace ct = clang::tooling;
namespace lc = llvm::cl;

static lc::OptionCategory toolOptions("Tool Options");
static lc::opt<bool> clProcessHeaders("I",
  lc::cat(toolOptions));
static lc::opt<bool> clVisitVarDecl("varDecl", lc::cat(toolOptions),
  lc::init(false));
static lc::opt<bool> clVisitFunctionDecl("functionDecl", lc::cat(toolOptions),
  lc::init(false));

void printVarDecl(clang::ASTContext* astContext, clang::VarDecl* varDecl) {
	auto& sourceManager = astContext->getSourceManager();

	llvm::outs() << std::format("{:*<80s}\n", "");
	llvm::outs() << "varDecl\n";

	llvm::outs() << std::format("getLocation(): {}\n",
	  locationToString(sourceManager, varDecl->getLocation()), true);

	llvm::outs() << std::format("variable name: {}\n",
	  varDecl->getQualifiedNameAsString());

	clang::SourceRange allSourceRange = varDecl->getSourceRange();
	assert(allSourceRange.isValid());
	assert(allSourceRange.getBegin() == varDecl->getBeginLoc());
	assert(allSourceRange.getEnd() == varDecl->getEndLoc());
	llvm::outs() << std::format("getSourceRange() [source]: {}\n{}",
	  rangeToString(sourceManager, allSourceRange, false),
	  getSourceTextWithLineNumbers(sourceManager, allSourceRange));
}

void printFunctionDecl(clang::ASTContext* astContext,
  clang::FunctionDecl* funcDecl) {
	auto& sourceManager = astContext->getSourceManager();

	llvm::outs() << std::format("{:*<80s}\n", "");

	llvm::outs() << std::format("getLocation() [function location]: {}\n",
	  locationToString(sourceManager, funcDecl->getLocation()), true);

	llvm::outs() << std::format("function name: {}\n",
	  funcDecl->getQualifiedNameAsString());

	auto templatedKind = funcDecl->getTemplatedKind();
	llvm::outs() << std::format("templated kind: {}\n",
	  functionDeclTemplatedKindToString(templatedKind));
	bool isDef = funcDecl->isThisDeclarationADefinition();
	llvm::outs() << std::format("definition: {}\n", isDef);

	clang::SourceRange allSourceRange = funcDecl->getSourceRange();
	assert(allSourceRange.isValid());
	assert(allSourceRange.getBegin() == funcDecl->getBeginLoc());
	assert(allSourceRange.getEnd() == funcDecl->getEndLoc());
	llvm::outs() << std::format("getSourceRange() [source]: {}\n{}",
	  rangeToString(sourceManager, allSourceRange, false),
	  getSourceTextWithLineNumbers(sourceManager, allSourceRange));

#if 1
	llvm::outs() << std::format("getLocation():\n    {}\n",
	  locationToString(sourceManager, funcDecl->getLocation())
	  );
#endif

	// Use getTypeSourceInfo from clang::DeclaratorDecl base class
	auto typeSourceInfo = funcDecl->getTypeSourceInfo();
	auto typeLoc = typeSourceInfo->getTypeLoc();
	auto returnLoc = typeLoc.getAs<clang::FunctionTypeLoc>().getReturnLoc();
	assert(typeLoc.getSourceRange().getBegin() == typeLoc.getBeginLoc());
	assert(typeLoc.getSourceRange().getEnd() == typeLoc.getEndLoc());

	clang::SourceRange returnTypeSourceRange =
	  funcDecl->getReturnTypeSourceRange();
	if (returnTypeSourceRange.isValid()) {
		llvm::outs() << std::format(
		  "getReturnTypeSourceRange() [return type]: {}\n{}",
		  rangeToString(sourceManager, funcDecl->getReturnTypeSourceRange(),
		  false),
		  addLineNumbers(getSourceText(sourceManager,
		  funcDecl->getReturnTypeSourceRange()),
		  sourceManager.getSpellingLineNumber(
		  funcDecl->getReturnTypeSourceRange().getBegin()),
		  sourceManager.getSpellingColumnNumber(
		  funcDecl->getReturnTypeSourceRange().getBegin())));
	} else {
		llvm::outs() << "no return type\n";
	}

	clang::SourceRange parametersSourceRange{
	  funcDecl->getParametersSourceRange()};
	if (parametersSourceRange.isValid()) {
		llvm::outs() << std::format(
		  "getParametersSourceRange() [parameters]: {}\n{}",
		  rangeToString(sourceManager, parametersSourceRange, false),
		  getSourceTextWithLineNumbers(sourceManager, parametersSourceRange));
	} else {
		llvm::outs() << "no parameters\n";
	}

	clang::SourceRange exceptSpecSourceRange{
	  funcDecl->getExceptionSpecSourceRange()};
	if (exceptSpecSourceRange.isValid()) {
		llvm::outs() << std::format(
		  "getExceptionSpecSourceRange() [exception specifier]: {}\n{}",
		  rangeToString(sourceManager, exceptSpecSourceRange, false),
		  getSourceTextWithLineNumbers(sourceManager, exceptSpecSourceRange));
	} else {
		llvm::outs() << "no exceptions specifier\n";
	}

	clang::SourceLocation sourceLocation{funcDecl->getPointOfInstantiation()};
	if (sourceLocation.isValid()) {
		llvm::outs() << std::format("getPointOfInstantiation():\n    {}",
		  locationToString(sourceManager, sourceLocation));
	} else {
		llvm::outs() << "no point of instantiation\n";
	}

	sourceLocation = funcDecl->getEllipsisLoc();
	if (sourceLocation.isValid()) {
		llvm::outs() << std::format("getEllipsisLoc() [ellipsis]: {}\n{}",
		  locationToString(sourceManager, funcDecl->getEllipsisLoc(), false),
		  getSourceTextWithLineNumbers(sourceManager,
		  clang::SourceRange(funcDecl->getEllipsisLoc(),
		  funcDecl->getEllipsisLoc()))
		  );
	} else {
		llvm::outs() << "no ellipsis\n";
	}

	clang::SourceRange typeRange{typeLoc.getSourceRange()};
	if (typeRange.isValid()) {
		llvm::outs() << std::format(
		  "getTypeSourceInfo().getTypeLoc().getSourceRange(): {}\n{}",
		  rangeToString(sourceManager, typeRange, false),
		  getSourceTextWithLineNumbers(sourceManager, typeRange)
		  );
	} else {
		llvm::outs() << "no source information\n";
	}

	clang::SourceRange retSourceRange{returnLoc.getSourceRange()};
	if (retSourceRange.isValid()) {
		llvm::outs() << std::format(
		  "getTypeSourceInfo().getTypeLoc().getAs<clang::FunctionTypeLoc>().getReturnLoc() [return type]: {}\n{}",
		  rangeToString(sourceManager, retSourceRange, false),
		  getSourceTextWithLineNumbers(sourceManager, retSourceRange)
		  );
	} else {
		llvm::outs() << "no return type from source info\n";
	}

	auto parensRange{typeLoc.getAs<clang::FunctionTypeLoc>().getParensRange()};
	if (parensRange.isValid()) {
		llvm::outs() << std::format(
		  "getTypeSourceInfo().getTypeLoc().getAs<clang::FunctionTypeLoc>().getParensRange() [parentheses]: {}\n{}",
		  rangeToString(sourceManager, parensRange, false),
		  getSourceTextWithLineNumbers(sourceManager, parensRange)
		  );
	} else {
		llvm::outs() << "no parentheses\n";
	}

	if (funcDecl->hasBody()) {
		clang::SourceRange bodyRange = funcDecl->getBody()->getSourceRange();
		llvm::outs() <<
		  std::format("getBody()->getSourceRange() [body]: {}\n{}",
		  rangeToString(sourceManager, bodyRange, false),
		  getSourceTextWithLineNumbers(sourceManager, bodyRange)
		  );
	}
}

class MyAstVisitor : public clang::RecursiveASTVisitor<MyAstVisitor> {
public:
	MyAstVisitor(clang::ASTContext& astContext) : astContext_(&astContext) {}
	bool VisitVarDecl(clang::VarDecl* varDecl) {
		if (!clVisitVarDecl) {
			return true;
		}
		auto& sourceManager = astContext_->getSourceManager();
		const auto& fileId = sourceManager.getFileID(varDecl->getLocation());
		if (clProcessHeaders || fileId == sourceManager.getMainFileID()) {
			printVarDecl(astContext_, varDecl);
		}
		return true;
	}
	bool VisitFunctionDecl(clang::FunctionDecl* funcDecl) {
		if (!clVisitFunctionDecl) {
			return true;
		}
		auto& sourceManager = astContext_->getSourceManager();
		const auto& fileId = sourceManager.getFileID(funcDecl->getLocation());
		if (clProcessHeaders || fileId == sourceManager.getMainFileID()) {
			printFunctionDecl(astContext_, funcDecl);
		}
		return true;
	}
	bool shouldVisitTemplateInstantiations() const {
		return true;
	}
private:
	clang::ASTContext* astContext_;
};

class MyAstConsumer : public clang::ASTConsumer {
public:
	void HandleTranslationUnit(clang::ASTContext& astContext) final {
		clang::TranslationUnitDecl* tuDecl =
		  astContext.getTranslationUnitDecl();
		MyAstVisitor visitor(astContext);
		visitor.TraverseDecl(tuDecl);
	}
};

class MyFrontendAction : public clang::ASTFrontendAction {
public:
	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
	  clang::CompilerInstance&, clang::StringRef) final {
		return std::unique_ptr<clang::ASTConsumer>{new MyAstConsumer};
	}
};

int main(int argc, char** argv) {
	auto expectedOptionsParser = ct::CommonOptionsParser::create(argc,
	  const_cast<const char**>(argv), toolOptions);
	if (!expectedOptionsParser) {
		llvm::errs() << std::format("Unable to create option parser ({}).\n",
		  llvm::toString(expectedOptionsParser.takeError()));
		return 1;
	}
	ct::CommonOptionsParser& optionsParser = *expectedOptionsParser;
	ct::ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());
	int status = tool.run(
	  ct::newFrontendActionFactory<MyFrontendAction>().get());
	if (status) {llvm::errs() << "error detected\n";}
	return !status ? 0 : 1;
}
