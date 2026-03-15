#include "llvm/Support/Error.h"

llvm::Expected<std::string> mayFail(bool shouldFail) {
	if (shouldFail) {
		return llvm::make_error<llvm::StringError>("oops",
		  llvm::inconvertibleErrorCode());
	} else {return "hello";}
}

void doWork1() {
	llvm::Expected<std::string> valueOrErr = mayFail(false);
	// ... (code does not check validity of valueOrErr)
} // ERROR: destructor for valueOrErr will abort program

void doWork2() {
	llvm::Expected<std::string> valueOrErr = mayFail(true);
	if (!valueOrErr) {
		// ... (code does not consume error)
	}
} // ERROR: destructor for valueOrErr will abort program

void doWork3() {
	llvm::Expected<std::string> valueOrErr = mayFail(true);
	if (!valueOrErr) {
		llvm::consumeError(valueOrErr.takeError());
		// ...
	}
} // OK: valueOrErr checked and (in error case) consumed
