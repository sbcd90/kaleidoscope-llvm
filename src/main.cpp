#include <iostream>
#include "Parser.h"

int main() {
    binOpPrecedence['<'] = 10;
    binOpPrecedence['+'] = 20;
    binOpPrecedence['-'] = 20;
    binOpPrecedence['*'] = 40;

    std::cout << "ready> ";
    getNextToken();
    mainLoop();
    return 0;
}
