#include <iostream>
#include "llvm/Support/TargetSelect.h"
#include "Parser.h"

int main() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    auto llvmContext = std::make_shared<LLVMContext>();
    binOpPrecedence['<'] = 10;
    binOpPrecedence['+'] = 20;
    binOpPrecedence['-'] = 20;
    binOpPrecedence['*'] = 40;

    std::cout << "ready> ";
    getNextToken();
    mainLoop(llvmContext);

    llvmContext->getModule()->print(llvm::errs(), nullptr);
    return 0;
}
