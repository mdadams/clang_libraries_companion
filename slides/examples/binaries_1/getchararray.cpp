#include <format>
#include <string>
#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/Error.h>

llvm::Expected<llvm::object::SymbolRef> findSymbol(
  llvm::object::ObjectFile& objFile, llvm::StringRef name) {
	for (const auto& sym : objFile.symbols()) {
		auto nameOrErr = sym.getName();
		if (!nameOrErr) {return nameOrErr.takeError();}
		if (*nameOrErr == name) {return sym;}
	}
	return llvm::make_error<llvm::StringError>("symbol not found",
	  llvm::inconvertibleErrorCode());
}

llvm::Expected<std::string> getCharArray(llvm::StringRef path,
  llvm::StringRef symName) {
	auto owningObjFileOrErr = llvm::object::ObjectFile::createObjectFile(path);
	if (!owningObjFileOrErr) {return owningObjFileOrErr.takeError();}
	auto objFile = owningObjFileOrErr->getBinary();
	auto symOrErr = findSymbol(*objFile, symName);
	if (!symOrErr) {return symOrErr.takeError();}
	auto addrOrErr = symOrErr->getAddress();
	if (!addrOrErr) {return addrOrErr.takeError();}
	llvm::Expected<llvm::object::section_iterator> sectOrErr =
	  symOrErr->getSection();
	if (!sectOrErr) {return sectOrErr.takeError();}
	if (*sectOrErr == objFile->section_end()) {
		return llvm::make_error<llvm::StringError>("symbol has no section",
		  llvm::inconvertibleErrorCode());
	}
	const llvm::object::SectionRef& sect = **sectOrErr;
	uint64_t offset = *addrOrErr - sect.getAddress();
	llvm::Expected<llvm::StringRef> contentsOrErr = sect.getContents();
	if (!contentsOrErr) {return contentsOrErr.takeError();}
	llvm::StringRef result =
	  contentsOrErr->drop_front(offset).split('\0').first;
	return std::string(result);
}

int main(int argc, char** argv) {
	if (argc < 3) {
		const char* argv0 = (argc >= 1) ? argv[0] : "getchararray";
		llvm::errs() << std::format("usage: {} objectfile symbol\n", argv0);
		return 1;
	}
	const char* path = argv[1];
	const char* symName = argv[2];
	auto stringOrErr = getCharArray(path, symName);
	if (!stringOrErr) {
		llvm::errs() << std::format("cannot get string {}\n",
		  llvm::toString(stringOrErr.takeError()));
		return 1;
	}
	llvm::outs() << std::format("string:\n{}\n", *stringOrErr);
	return 0;
}
