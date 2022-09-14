#include <iostream>
#include "llvm/Support/TargetSelect.h"
#include "Parser.h"

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// putchard - putchar that takes a double and returns 0.
extern "C" DLLEXPORT double putchard(double X) {
    fputc((char)X, stderr);
    return 0;
}

extern "C" DLLEXPORT double printd(double X) {
    fprintf(stderr, "%f\n", X);
    return 0;
}

int main() {
/*    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();*/

    std::cout << "ready> ";
    getNextToken();

    auto llvmContext = std::make_shared<LLVMContext>();

    mainLoop(llvmContext);

//    llvmContext->getModule()->print(llvm::errs(), nullptr);
    llvmContext->initializeTargetRegistry();
    return 0;
}
