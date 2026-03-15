#include <algorithm>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/Format.h>
#include <llvm/Support/raw_ostream.h>

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

std::optional<llvm::object::SymbolRef> getNextSymbol(
  const std::vector<llvm::object::SymbolRef>& sortedSymbols,
  const llvm::object::SymbolRef& targetSym) {
	uint64_t targetAddr = llvm::cantFail(targetSym.getAddress());
	auto iter = std::upper_bound(sortedSymbols.begin(), sortedSymbols.end(),
	  targetAddr, [](uint64_t addr, const llvm::object::SymbolRef& s) {
		return addr < llvm::cantFail(s.getAddress());
	});
	if (iter != sortedSymbols.end()) {return *iter;}
	return std::nullopt;
}

std::vector<llvm::object::SymbolRef> getSortedSymbolsInSection(
  const llvm::object::SectionRef &sect) {
	std::vector<llvm::object::SymbolRef> syms;
	const llvm::object::ObjectFile *objFile = sect.getObject();
	for (const llvm::object::SymbolRef& sym : objFile->symbols()) {
		auto secIterOrErr = sym.getSection();
		if (!secIterOrErr) {
			llvm::consumeError(secIterOrErr.takeError());
			continue;
		}
		if (*secIterOrErr == sect) {syms.push_back(sym);}
	}
	std::sort(syms.begin(), syms.end(),
	  [](const llvm::object::SymbolRef& a, const llvm::object::SymbolRef& b) {
		return llvm::cantFail(a.getAddress()) < llvm::cantFail(b.getAddress());
	  });
	return syms;
}

llvm::Error printSymInfo(llvm::StringRef path, llvm::StringRef symName) {
	auto owningObjFileOrErr = llvm::object::ObjectFile::createObjectFile(path);
	if (!owningObjFileOrErr) {return owningObjFileOrErr.takeError();}
	auto objFile = owningObjFileOrErr->getBinary();
	auto symOrErr = findSymbol(*objFile, symName);
	if (!symOrErr) {return symOrErr.takeError();}
	auto symAddrOrErr = symOrErr->getAddress();
	if (!symAddrOrErr) {return symAddrOrErr.takeError();}
	auto sectOrErr = symOrErr->getSection();
	if (!sectOrErr) {return sectOrErr.takeError();}
	if (*sectOrErr == objFile->section_end()) {
		return llvm::make_error<llvm::StringError>("symbol has no section",
		  llvm::inconvertibleErrorCode());
	}
	const llvm::object::SectionRef& sect = **sectOrErr;
	auto sectDataOrErr = sect.getContents();
	if (!sectDataOrErr) {return sectDataOrErr.takeError();}
	uint64_t sectAddr = sect.getAddress();
	uint64_t sectSize = sect.getSize();
	uint64_t symAlign = symOrErr->getAlignment() ? symOrErr->getAlignment() :
	  sect.getAlignment().value();
	auto sectNameOrErr = sect.getName();
	if (!sectNameOrErr) {return sectNameOrErr.takeError();}
	uint64_t startOff = *symAddrOrErr - sectAddr;
	uint64_t endOff = sectSize;
	auto symList = getSortedSymbolsInSection(sect);
	auto maybeNextSym = getNextSymbol(symList, *symOrErr);
	if (maybeNextSym) {
		auto nextSymAddrOrErr = maybeNextSym->getAddress();
		if (nextSymAddrOrErr) {endOff = *nextSymAddrOrErr - sectAddr;}
		else {llvm::consumeError(nextSymAddrOrErr.takeError());}
	}
	llvm::StringRef symData = sectDataOrErr->slice(startOff, endOff);
	llvm::outs() << std::format("format: {}\n" "symbol name: {}\n"
	  "section name: {}\n" "section address: {}\n" "section size: {}\n"
	  "section offset: {}\n" "symbol address: {}\n" "symbol alignment: {}\n"
	  "symbol size (estimate): {}\n",
	  std::string_view(objFile->getFileFormatName()),
	  std::string_view(symName), std::string_view(*sectNameOrErr),
	  sectAddr, sectSize, startOff, *symAddrOrErr, symAlign, symData.size());
	llvm::outs() << llvm::format_bytes_with_ascii(
	  llvm::ArrayRef(reinterpret_cast<const uint8_t*>(symData.data()),
	  symData.size()), *symAddrOrErr) << '\n';
	return llvm::Error::success();
}

int main(int argc, char** argv) {
	if (argc < 3) {
		const char* argv0 = (argc >= 1) ? argv[0] : "printsyminfo";
		llvm::errs() << std::format("usage: {} objectFile symbol\n", argv0);
		return 1;
	}
	const char* path = argv[1];
	const char* symName = argv[2];
	if (auto err = printSymInfo(path, symName)) {
		llvm::errs() << std::format("error: {}\n",
		  llvm::toString(std::move(err)));
		return 1;
	}
	return 0;
}
