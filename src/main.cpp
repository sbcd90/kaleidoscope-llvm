#include <iostream>
#include "Parser.h"
#include "LLVM.h"

int main() {
    binOpPrecedence['<'] = 10;
    binOpPrecedence['+'] = 20;
    binOpPrecedence['-'] = 20;
    binOpPrecedence['*'] = 40;

    std::cout << "ready> ";
    getNextToken();

    initializeModule();
    mainLoop();

    theModule->print(llvm::errs(), nullptr);
    return 0;
}
