#include "Parser.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Verifier.h"

static llvm::Value* logErrorV(const char *str) {
    logError(str);
    return nullptr;
}

llvm::Value* ast::NumberExprAST::codegen() {
    return llvm::ConstantFP::get(*llvmContext->getContext(), llvm::APFloat{val});
}

llvm::Value* ast::VariableExprAST::codegen() {
    auto v = namedValues[name];
    if (!v) {
        return logErrorV("Unknown variable name");
    }
    return v;
}

llvm::Value* ast::UnaryExprAST::codegen() {
    auto operandV = operand->codegen();
    if (!operandV) {
        return nullptr;
    }

    auto f = getFunction(llvmContext, std::string{"unary"} + op);
    if (!f) {
        return logErrorV("Unknown unary operator");
    }
    return llvmContext->getBuilder()->CreateCall(f, operandV, "unop");
}

llvm::Value* ast::BinaryExprAST::codegen() {
    auto l = lhs->codegen();
    auto r = rhs->codegen();

    if (!l || !r) {
        return nullptr;
    }

    switch (op) {
        case '+':
            return llvmContext->getBuilder()->CreateFAdd(l, r, "addtmp");
        case '-':
            return llvmContext->getBuilder()->CreateFSub(l, r, "subtmp");
        case '*':
            return llvmContext->getBuilder()->CreateFMul(l, r, "multmp");
        case '<':
            l = llvmContext->getBuilder()->CreateFCmpULT(l, r, "cmptmp");
            return llvmContext->getBuilder()->CreateUIToFP(l, llvm::Type::getDoubleTy(*llvmContext->getContext()), "booltmp");
        default:
            break;
    }

    using namespace std::string_literals;
    auto f = getFunction(llvmContext, "binary"s + op);
    assert(f && "binary operator not found!");

    llvm::Value *ops[2] = {l, r};
    return llvmContext->getBuilder()->CreateCall(f, ops, "binop");
}

llvm::Value* ast::CallexprAST::codegen() {
    auto calleeF = ast::getFunction(llvmContext, callee);
    if (!calleeF) {
        return logErrorV("Unknown function referenced");
    }

    if (calleeF->arg_size() != args.size()) {
        return logErrorV("Incorrect # arguments passed");
    }

    std::vector<llvm::Value*> argsV{};
    int e = args.size();

    for (int i = 0; i < e; ++i) {
        argsV.push_back(args[i]->codegen());
        if (!argsV.back()) {
            return nullptr;
        }
    }

    return llvmContext->getBuilder()->CreateCall(calleeF, argsV, "calltmp");
}

llvm::Value* ast::IfExprAST::codegen() {
    auto condV = cond->codegen();
    if (!condV) {
        return nullptr;
    }

    condV = llvmContext->getBuilder()->CreateFCmpONE(condV, llvm::ConstantFP::get(*(llvmContext->getContext()), llvm::APFloat(0.0)), "ifcond");

    auto theFunction = llvmContext->getBuilder()->GetInsertBlock()->getParent();

    auto thenBB = llvm::BasicBlock::Create(*(llvmContext->getContext()), "then", theFunction);
    auto elseBB = llvm::BasicBlock::Create(*(llvmContext->getContext()), "else");
    auto mergeBB = llvm::BasicBlock::Create(*(llvmContext->getContext()), "ifcont");

    llvmContext->getBuilder()->CreateCondBr(condV, thenBB, elseBB);

    llvmContext->getBuilder()->SetInsertPoint(thenBB);

    auto thenV = then->codegen();
    if (!thenV) {
        return nullptr;
    }

    llvmContext->getBuilder()->CreateBr(mergeBB);
    thenBB = llvmContext->getBuilder()->GetInsertBlock();

    theFunction->getBasicBlockList().push_back(elseBB);
    llvmContext->getBuilder()->SetInsertPoint(elseBB);

    auto elseV = else_->codegen();
    if (!elseV) {
        return nullptr;
    }

    llvmContext->getBuilder()->CreateBr(mergeBB);
    elseBB = llvmContext->getBuilder()->GetInsertBlock();

    theFunction->getBasicBlockList().push_back(mergeBB);
    llvmContext->getBuilder()->SetInsertPoint(mergeBB);

    auto pn = llvmContext->getBuilder()->CreatePHI(llvm::Type::getDoubleTy(*(llvmContext->getContext())), 2, "iftmp");

    pn->addIncoming(thenV, thenBB);
    pn->addIncoming(elseV, elseBB);
    return pn;
}

llvm::Value* ast::ForExprAST::codegen() {
    auto startVal = start->codegen();
    if (!startVal) {
        return nullptr;
    }

    auto theFunction = llvmContext->getBuilder()->GetInsertBlock()->getParent();
    auto preHeaderBB = llvmContext->getBuilder()->GetInsertBlock();
    auto loopBB = llvm::BasicBlock::Create(*(llvmContext->getContext()), "loop", theFunction);

    llvmContext->getBuilder()->CreateBr(loopBB);
    llvmContext->getBuilder()->SetInsertPoint(loopBB);

    auto variable = llvmContext->getBuilder()->CreatePHI(llvm::Type::getDoubleTy(*(llvmContext->getContext())), 2, varName);
    variable->addIncoming(startVal, preHeaderBB);

    auto oldVal = ast::namedValues[varName];
    ast::namedValues[varName] = variable;

    if (!body->codegen()) {
        return nullptr;
    }

    llvm::Value *stepVal;
    if (step) {
        stepVal = step->codegen();
        if (!stepVal) {
            return nullptr;
        }
    } else {
        stepVal = llvm::ConstantFP::get(*(llvmContext->getContext()), llvm::APFloat(1.0));
    }

    auto nextVar = llvmContext->getBuilder()->CreateFAdd(variable, stepVal, "nextvar");

    auto endCond = end->codegen();
    if (!endCond) {
        return nullptr;
    }

    endCond = llvmContext->getBuilder()->CreateFCmpONE(endCond, llvm::ConstantFP::get(*(llvmContext->getContext()), llvm::APFloat(0.0)), "loopcond");

    auto loopEndBB = llvmContext->getBuilder()->GetInsertBlock();
    auto afterBB = llvm::BasicBlock::Create(*(llvmContext->getContext()), "afterloop", theFunction);

    llvmContext->getBuilder()->CreateCondBr(endCond, loopBB, afterBB);
    llvmContext->getBuilder()->SetInsertPoint(afterBB);

    variable->addIncoming(nextVar, loopEndBB);

    if (oldVal) {
        namedValues[varName] = oldVal;
    } else {
        namedValues.erase(varName);
    }

    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*(llvmContext->getContext())));
}

llvm::Function* ast::PrototypeAST::codegen() {
    std::vector<llvm::Type*> doubles{args.size(), llvm::Type::getDoubleTy(*llvmContext->getContext())};
    auto ft = llvm::FunctionType::get(llvm::Type::getDoubleTy(*llvmContext->getContext()), doubles, false);

    auto f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, llvmContext->getModule().get());

    auto idx = 0;
    for (auto &arg: f->args()) {
        arg.setName(args[idx++]);
    }
    return f;
}

llvm::Function* ast::FunctionAST::codegen() {
    auto &p = *proto;
    functionProtos[proto->getName()] = std::move(proto);
    auto *theFunction = ast::getFunction(llvmContext, p.getName());

    if (!theFunction) {
        return nullptr;
    }

    if (p.isBinaryOp()) {
        llvmContext->addToBinOpPrecedence(std::make_pair(p.getOperatorName(), p.getBinaryPrecedence()));
    }

    auto bb = llvm::BasicBlock::Create(*llvmContext->getContext(), "entry", theFunction);
    llvmContext->getBuilder()->SetInsertPoint(bb);

    namedValues.clear();
    for (auto &arg: theFunction->args()) {
        namedValues[std::string{arg.getName()}] = &arg;
    }

    if (auto retVal = body->codegen()) {
        llvmContext->getBuilder()->CreateRet(retVal);
        llvm::verifyFunction(*theFunction);
        llvmContext->getFPM()->run(*theFunction);
        return theFunction;
    }

    theFunction->eraseFromParent();
    return nullptr;
}