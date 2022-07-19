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
            return llvmContext->getBuilder()->CreateFSub(l, r, "multmp");
        case '<':
            l = llvmContext->getBuilder()->CreateFCmpULT(l, r, "cmptmp");
            return llvmContext->getBuilder()->CreateUIToFP(l, llvm::Type::getDoubleTy(*llvmContext->getContext()), "booltmp");
        default:
            return logErrorV("invalid binary operator");
    }
}

llvm::Value* ast::CallexprAST::codegen() {
    auto calleeF = llvmContext->getModule()->getFunction(callee);
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
    auto *theFunction = llvmContext->getModule()->getFunction(proto->getName());

    if (!theFunction) {
        theFunction = proto->codegen();
    }

    if (!theFunction) {
        return nullptr;
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
        return theFunction;
    }

    theFunction->eraseFromParent();
    return nullptr;
}