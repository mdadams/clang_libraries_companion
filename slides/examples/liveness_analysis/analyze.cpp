#include <clang/AST/ASTContext.h>
#include <clang/Analysis/CFG.h>
#include <clang/Analysis/AnalysisDeclContext.h>
#include <clang/Analysis/Analyses/LiveVariables.h>

void analyzeFunc(clang::ASTContext& astContext, const clang::FunctionDecl*
  funcDecl, bool printCfg) {
	clang::AnalysisDeclContextManager adcm(astContext);
	clang::AnalysisDeclContext *adc = adcm.getContext(
	  llvm::cast<clang::Decl>(funcDecl));
	assert(adc);
	adc->getCFGBuildOptions().setAllAlwaysAdd();
	const clang::CFG& cfg = *adc->getCFG();
	if (printCfg)
	  {cfg.print(llvm::outs(), astContext.getLangOpts(), false);}
	clang::LiveVariables *lv = adc->getAnalysis<clang::LiveVariables>();
	if (!lv) {return;}
	auto observer = std::make_unique<clang::LiveVariables::Observer>();
	assert(observer);
	lv->runOnAllBlocks(*observer);
	lv->dumpBlockLiveness((funcDecl->getASTContext()).getSourceManager());
}
