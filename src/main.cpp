#include <iostream>
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
    auto ksDebugInfo = std::make_shared<ast::DebugInfo>(llvmContext);

    ksDebugInfo->theCU = llvmContext->getDBuilder()->createCompileUnit(llvm::dwarf::DW_LANG_C, llvmContext->getDBuilder()->createFile("fib.ks", "/home/sbcd90/Documents/programs/kaleidoscope-llvm/src"),
                                                     "Kaleidoscope Compiler", false, "", 0);

    mainLoop(llvmContext, ksDebugInfo);

//    llvmContext->getModule()->print(llvm::errs(), nullptr);
    llvmContext->initializeTargetRegistry();
    llvmContext->getDBuilder()->finalize();
    return 0;
}
