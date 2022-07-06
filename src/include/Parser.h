#include "Ast.h"
#include "Lexer.h"
#include <map>

static int curTok = 1;

static int getNextToken() {
    return curTok = getTok();
}

static std::map<char, int> binOpPrecedence;

std::unique_ptr<ast::ExprAST> logError(std::string s) {
    std::cout << "Error: " << s << std::endl;
    return nullptr;
}

std::unique_ptr<ast::PrototypeAST> logErrorP(std::string s) {
    logError(s);
    return nullptr;
}

static std::unique_ptr<ast::PrototypeAST> parsePrototype() {
    using namespace std::string_literals;
    if (curTok != tokIdentifier) {
        return logErrorP("Expected function name in prototype"s);
    }

    auto fnName = identifierStr;
    getNextToken();

    if (curTok != '(') {
        logErrorP("Expected '(' in prototype"s);
    }

    std::vector<std::string> argNames{};
    while (getNextToken() == tokIdentifier) {
        argNames.push_back(identifierStr);
    }

    if (curTok != ')') {
        logErrorP("Expected ')' in prototype"s);
    }
    getNextToken();
    return std::make_unique<ast::PrototypeAST>(fnName, std::move(argNames));
}

static std::unique_ptr<ast::FunctionAST> parseDefinition() {
    getNextToken();
    auto proto = parsePrototype();
    if (proto == nullptr) {
        return nullptr;
    }
}

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