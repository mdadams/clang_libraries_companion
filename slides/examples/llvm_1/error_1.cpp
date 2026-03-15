#include <utility>
#include "llvm/Support/Error.h"

llvm::Error mayFail(bool shouldFail) {
	return shouldFail ? llvm::make_error<llvm::StringError>("oops",
	  llvm::inconvertibleErrorCode()) : llvm::Error::success();
}

void doWork1() {
	llvm::Error err = mayFail(false);
	// ... (code does not check validity of err)
} // ERROR: destructor for err will abort program

void doWork2() {
	llvm::Error err = mayFail(true);
	if (err) {
		// ... (code does not consume error)
	}
} // ERROR: destructor for err will abort program

void doWork3() {
	llvm::Error err = mayFail(true);
	if (err) {
		llvm::consumeError(std::move(err));
		// ...
	}
} // OK: err checked and (in error case) consumed
