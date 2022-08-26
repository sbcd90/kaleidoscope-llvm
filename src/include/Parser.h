#include "Ast.h"
#include "Lexer.h"
#include <map>
#include <iostream>

static int curTok = 1;

static int getNextToken() {
    return curTok = getTok();
}

static int getTokPrecedence(const std::shared_ptr<LLVMContext> &llvmContext) {
    if (!isascii(curTok)) {
        return -1;
    }

    if (llvmContext->getBinOpPrecedence().find(curTok) == llvmContext->getBinOpPrecedence().end()) {
        return -1;
    }
    auto tokPrec = llvmContext->getBinOpPrecedence().at(curTok);
    if (tokPrec <= 0) {
        return -1;
    }
    return tokPrec;
}

static std::unique_ptr<ast::ExprAST> logError(std::string s) {
    std::cout << "Error: " << s << std::endl;
    return nullptr;
}

static std::unique_ptr<ast::PrototypeAST> logErrorP(std::string s) {
    logError(s);
    return nullptr;
}

static std::unique_ptr<ast::ExprAST> parseExpression(const std::shared_ptr<LLVMContext> &llvmContext);

static std::unique_ptr<ast::ExprAST> parseNumberExpr(const std::shared_ptr<LLVMContext> &llvmContext) {
    auto result = std::make_unique<ast::NumberExprAST>(numVal, llvmContext);
    getNextToken();
    return std::move(result);
}

static std::unique_ptr<ast::ExprAST> parseParenExpr(const std::shared_ptr<LLVMContext> &llvmContext) {
    getNextToken();
    auto v = parseExpression(llvmContext);
    if (!v) {
        return nullptr;
    }

    if (curTok != ')') {
        return logError("expected ')'");
    }
    getNextToken();
    return v;
}

static std::unique_ptr<ast::ExprAST> parseIdentifierExpr(const std::shared_ptr<LLVMContext> &llvmContext) {
    auto idName = identifierStr;
    getNextToken();

    if (curTok != '(') {
        return std::make_unique<ast::VariableExprAST>(idName, llvmContext);
    }

    getNextToken();
    std::vector<std::unique_ptr<ast::ExprAST>> args{};

    if (curTok != ')') {
        while (true) {
            if (auto arg = parseExpression(llvmContext)) {
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
    return std::make_unique<ast::CallexprAST>(idName, std::move(args), llvmContext);
}

static std::unique_ptr<ast::ExprAST> parseIfExpr(const std::shared_ptr<LLVMContext> &llvmContext) {
    getNextToken();

    auto cond = parseExpression(llvmContext);
    if (!cond) {
        return nullptr;
    }

    if (curTok != tokThen) {
        return logError("expected then");
    }
    getNextToken();

    auto then = parseExpression(llvmContext);
    if (!then) {
        return nullptr;
    }

    if (curTok != tokElse) {
        return logError("expected else");
    }

    getNextToken();

    auto else_ = parseExpression(llvmContext);
    if (!else_) {
        return nullptr;
    }

    return std::make_unique<ast::IfExprAST>(std::move(cond), std::move(then), std::move(else_), llvmContext);
}

static std::unique_ptr<ast::ExprAST> parseForExpr(const std::shared_ptr<LLVMContext> &llvmContext) {
    getNextToken();

    if (curTok != tokIdentifier) {
        return logError("expected identifier after for");
    }

    auto idName = identifierStr;
    getNextToken();

    if (curTok != '=') {
        return logError("expected '=' after for");
    }
    getNextToken();

    auto start = parseExpression(llvmContext);
    if (!start) {
        return nullptr;
    }
    if (curTok != ',') {
        return logError("expected ',' after for start value");
    }
    getNextToken();

    auto end = parseExpression(llvmContext);
    if (!end) {
        return nullptr;
    }

    std::unique_ptr<ast::ExprAST> step;
    if (curTok == ',') {
        getNextToken();
        step = parseExpression(llvmContext);

        if (!step) {
            return nullptr;
        }
    }

    if (curTok != tokIn) {
        return logError("expected 'in' after for");
    }
    getNextToken();

    auto body = parseExpression(llvmContext);
    if (!body) {
        return nullptr;
    }

    return std::make_unique<ast::ForExprAST>(idName, std::move(start), std::move(end), std::move(step), std::move(body), llvmContext);
}

static std::unique_ptr<ast::ExprAST> parsePrimary(const std::shared_ptr<LLVMContext> &llvmContext) {
    switch (curTok) {
        default:
            return logError("unknown token when expecting an expression");
        case tokIdentifier:
            return parseIdentifierExpr(llvmContext);
        case tokNumber:
            return parseNumberExpr(llvmContext);
        case '(':
            return parseParenExpr(llvmContext);
        case tokIf:
            return parseIfExpr(llvmContext);
        case tokFor:
            return parseForExpr(llvmContext);
    }
}

static std::unique_ptr<ast::ExprAST> parseBinOpRHS(int exprPrec,
                                                   std::unique_ptr<ast::ExprAST> lhs,
                                                   const std::shared_ptr<LLVMContext> &llvmContext) {
    while (true) {
        auto tokPrec = getTokPrecedence(llvmContext);

        if (tokPrec < exprPrec) {
            return lhs;
        }
        auto binOp = curTok;
        getNextToken();

        auto rhs = parsePrimary(llvmContext);
        if (!rhs) {
            return nullptr;
        }

        auto nextPrec = getTokPrecedence(llvmContext);
        if (tokPrec < nextPrec) {
            rhs = parseBinOpRHS(tokPrec+1, std::move(rhs), llvmContext);
            if (!rhs) {
                return nullptr;
            }
        }

        lhs = std::make_unique<ast::BinaryExprAST>(binOp, std::move(lhs), std::move(rhs), llvmContext);
    }
}

static std::unique_ptr<ast::ExprAST> parseExpression(const std::shared_ptr<LLVMContext> &llvmContext) {
    auto lhs = parsePrimary(llvmContext);
    if (!lhs) {
        return nullptr;
    }
    return parseBinOpRHS(0, std::move(lhs), llvmContext);
}

static std::unique_ptr<ast::PrototypeAST> parsePrototype(const std::shared_ptr<LLVMContext> &llvmContext) {
    using namespace std::string_literals;
    std::string fnName;

    unsigned kind = 0;
    unsigned binaryPrecedence = 30;

    switch (curTok) {
        case tokIdentifier:
            fnName = identifierStr;
            kind = 0;
            getNextToken();
            break;
        case tokBinary:
            getNextToken();
            if (!isascii(curTok)) {
                return logErrorP("Expected binary operator"s);
            }
            fnName = "binary";
            fnName += (char) curTok;
            kind = 2;

            getNextToken();
            if (curTok == tokNumber) {
                if (numVal < 1 || numVal > 100) {
                    return logErrorP("Invalid precedence: must be 1..100"s);
                }
                binaryPrecedence = (unsigned) numVal;
                getNextToken();
            }
            break;
    }

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

    if (kind && argNames.size() != kind) {
        return logErrorP("Invalid number of operands for operator"s);
    }
    return std::make_unique<ast::PrototypeAST>(fnName, std::move(argNames), llvmContext, kind != 0, binaryPrecedence);
}

static std::unique_ptr<ast::FunctionAST> parseDefinition(const std::shared_ptr<LLVMContext> &llvmContext) {
    getNextToken();
    auto proto = parsePrototype(llvmContext);
    if (proto == nullptr) {
        return nullptr;
    }

    if (auto e = parseExpression(llvmContext)) {
        return std::make_unique<ast::FunctionAST>(std::move(proto), std::move(e), llvmContext);
    }
    return nullptr;
}

static std::unique_ptr<ast::FunctionAST> parseTopLevelExpr(const std::shared_ptr<LLVMContext> &llvmContext) {
    if (auto e = parseExpression(llvmContext)) {
        auto proto = std::make_unique<ast::PrototypeAST>("__anon_expr",
                                                         std::vector<std::string>{}, llvmContext);
        return std::make_unique<ast::FunctionAST>(std::move(proto), std::move(e), llvmContext);
    }
    return nullptr;
}

static std::unique_ptr<ast::PrototypeAST> parseExtern(const std::shared_ptr<LLVMContext> &llvmContext) {
    getNextToken();
    return parsePrototype(llvmContext);
}

static void handleDefinition(const std::shared_ptr<LLVMContext> &llvmContext) {
    if (auto fnAst = parseDefinition(llvmContext)) {
        if (auto *fnIR = fnAst->codegen()) {
/*            std::cout << "Read function definition:" << std::endl;
            fnIR->print(llvm::errs());
            std::cout << std::endl;*/
            llvmContext->handleDefinition();
        }
    } else {
        getNextToken();
    }
}

static void handleExtern(const std::shared_ptr<LLVMContext> &llvmContext) {
    if (auto protoAst = parseExtern(llvmContext)) {
        if (auto *fnIR = protoAst->codegen()) {
/*            std::cout << "Read extern: " << std::endl;
            fnIR->print(llvm::errs());
            std::cout << std::endl;*/
            ast::functionProtos[protoAst->getName()] = std::move(protoAst);
        }
    } else {
        getNextToken();
    }
}

static void handleTopLevelExpression(const std::shared_ptr<LLVMContext> &llvmContext) {
    if (auto fnAst = parseTopLevelExpr(llvmContext)) {
        if (auto *fnIR = fnAst->codegen()) {
/*            std::cout << "Read top-level expression:" << std::endl;
            fnIR->print(llvm::errs());

            std::cout << std::endl;*/

            llvmContext->handleTopLevelExprJit();
//            fnIR->eraseFromParent();
        }
    } else {
        getNextToken();
    }
}

static void mainLoop(const std::shared_ptr<LLVMContext> &llvmContext) {
    while (true) {
        std::cout << "ready> ";
        switch (curTok) {
            case tokEof:
                return;
            case ';':
                getNextToken();
                break;
            case tokDef:
                handleDefinition(llvmContext);
                break;
            case tokExtern:
                handleExtern(llvmContext);
                break;
            default:
                handleTopLevelExpression(llvmContext);
                break;
        }
    }
}