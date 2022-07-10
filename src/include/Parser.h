#include "Ast.h"
#include "Lexer.h"
#include <map>

static int curTok = 1;

static int getNextToken() {
    return curTok = getTok();
}

static std::map<char, int> binOpPrecedence;

static int getTokPrecedence() {
    if (!isascii(curTok)) {
        return -1;
    }

    auto tokPrec = binOpPrecedence[curTok];
    if (tokPrec <= 0) {
        return -1;
    }
    return tokPrec;
}

std::unique_ptr<ast::ExprAST> logError(std::string s) {
    std::cout << "Error: " << s << std::endl;
    return nullptr;
}

std::unique_ptr<ast::PrototypeAST> logErrorP(std::string s) {
    logError(s);
    return nullptr;
}

static std::unique_ptr<ast::ExprAST> parseExpression();

static std::unique_ptr<ast::ExprAST> parseNumberExpr() {
    auto result = std::make_unique<ast::NumberExprAST>(numVal);
    getNextToken();
    return std::move(result);
}

static std::unique_ptr<ast::ExprAST> parseParenExpr() {
    getNextToken();
    auto v = parseExpression();
    if (!v) {
        return nullptr;
    }

    if (curTok != ')') {
        return logError("expected ')'");
    }
    getNextToken();
    return v;
}

static std::unique_ptr<ast::ExprAST> parseIdentifierExpr() {
    auto idName = identifierStr;
    getNextToken();

    if (curTok != '(') {
        return std::make_unique<ast::VariableExprAST>(idName);
    }

    getNextToken();
    std::vector<std::unique_ptr<ast::ExprAST>> args{};

    if (curTok != ')') {
        while (true) {
            if (auto arg = parseExpression()) {
                args.push_back(std::move(arg));
            } else {
                return nullptr;
            }

            if (curTok == ')') {
                break;
            }
            if (curTok != ',') {
                return logError("Expected ')' or ',' in argument list");
            }
            getNextToken();
        }
    }

    getNextToken();
    return std::make_unique<ast::CallexprAST>(idName, std::move(args));
}

static std::unique_ptr<ast::ExprAST> parsePrimary() {
    switch (curTok) {
        default:
            return logError("unknown token when expecting an expression");
        case tokIdentifier:
            return parseIdentifierExpr();
        case tokNumber:
            return parseNumberExpr();
        case '(':
            return parseParenExpr();
    }
}

static std::unique_ptr<ast::ExprAST> parseBinOpRHS(int exprPrec,
                                                   std::unique_ptr<ast::ExprAST> lhs) {
    while (true) {
        auto tokPrec = getTokPrecedence();

        if (tokPrec < exprPrec) {
            return lhs;
        }
        auto binOp = curTok;
        getNextToken();

        auto rhs = parsePrimary();
        if (!rhs) {
            return nullptr;
        }

        auto nextPrec = getTokPrecedence();
        if (tokPrec < nextPrec) {
            rhs = parseBinOpRHS(tokPrec+1, std::move(rhs));
            if (!rhs) {
                return nullptr;
            }
        }

        lhs = std::make_unique<ast::BinaryExprAST>(binOp, std::move(lhs), std::move(rhs));
    }
}

static std::unique_ptr<ast::ExprAST> parseExpression() {
    auto lhs = parsePrimary();
    if (!lhs) {
        return nullptr;
    }
    return parseBinOpRHS(0, std::move(lhs));
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

    if (auto e = parseExpression()) {
        return std::make_unique<ast::FunctionAST>(std::move(proto), std::move(e));
    }
    return nullptr;
}

static std::unique_ptr<ast::FunctionAST> parseTopLevelExpr() {
    if (auto e = parseExpression()) {
        auto proto = std::make_unique<ast::PrototypeAST>("__anon_expr",
                                                         std::vector<std::string>{});
        return std::make_unique<ast::FunctionAST>(std::move(proto), std::move(e));
    }
    return nullptr;
}

static std::unique_ptr<ast::PrototypeAST> parseExtern() {
    getNextToken();
    return parsePrototype();
}

static void handleDefinition() {
    if (parseDefinition()) {
        std::cout << "Parsed a function definition." << std::endl;
    } else {
        getNextToken();
    }
}

static void handleExtern() {
    if (parseExtern()) {
        std::cout << "Parsed an extern" << std::endl;
    } else {
        getNextToken();
    }
}

static void handleTopLevelExpression() {
    if (parseTopLevelExpr()) {
        std::cout << "Parsed a top-level expr" << std::endl;
    } else {
        getNextToken();
    }
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