#include "Ast.h"
#include "Lexer.h"
#include <map>

static int curTok = 1;

static int getNextToken() {
    return curTok = getTok();
}

static std::map<char, int> binOpPrecedence;

static void handleDefinition() {

}

static void handleExtern() {

}

static void handleTopLevelExpression() {

}

static void mainLoop() {
    while (true) {
        std::cout << "ready> ";
        switch (curTok) {
            case tokEof:
                return;
            case ';':
                getNextToken();
                break;
            case tokDef:
                handleDefinition();
                break;
            case tokExtern:
                handleExtern();
                break;
            default:
                handleTopLevelExpression();
                break;
        }
    }
}