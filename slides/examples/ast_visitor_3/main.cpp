#include <format>
#include <vector>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

namespace ct = clang::tooling;

class MyAstVisitor : public clang::RecursiveASTVisitor<MyAstVisitor> {
public:
	MyAstVisitor(clang::ASTContext& astContext) : astContext_(&astContext),
	  stack_() {}
	bool TraverseCXXRecordDecl(clang::CXXRecordDecl* recDecl);
private:
	using Base = clang::RecursiveASTVisitor<MyAstVisitor>;
	void printStack() const;
	clang::ASTContext* astContext_;
	std::vector<const clang::CXXRecordDecl*> stack_;
};

bool MyAstVisitor::TraverseCXXRecordDecl(clang::CXXRecordDecl* recDecl) {
	clang::SourceManager& sourceManager = astContext_->getSourceManager();
	stack_.push_back(recDecl);
	bool result = Base::TraverseCXXRecordDecl(recDecl);
	if (sourceManager.getFileID(recDecl->getLocation()) ==
	  sourceManager.getMainFileID()) {printStack();}
	stack_.pop_back();
	return result;
}

void MyAstVisitor::printStack() const {
	std::string s;
	for (auto i = stack_.begin(); i != stack_.end(); ++i) {
		std::string name((*i)->getName());
		s += std::format("{}{}", i != stack_.begin() ? " -> " : "",
		  name.size() ? name : "(anonymous)");
	}
	llvm::outs() << s << '\n';
}

class MyAstConsumer : public clang::ASTConsumer {
public:
	void HandleTranslationUnit(clang::ASTContext& astContext) final {
		clang::TranslationUnitDecl* tuDecl =
		  astContext.getTranslationUnitDecl();
		MyAstVisitor astVisitor(astContext);
		astVisitor.TraverseDecl(tuDecl);
	}
};

class MyFrontendAction : public clang::ASTFrontendAction {
public:
	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
	  clang::CompilerInstance& compInstance, clang::StringRef) final {
		return std::unique_ptr<clang::ASTConsumer>{new MyAstConsumer};
	}
};

static llvm::cl::OptionCategory toolOptions("Tool Options");

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
