#include "Debugger.h"

llvm::DIType* ast::DebugInfo::getDoubleTy() {
    if (dblTy) {
        return dblTy;
    }
    dblTy = llvmContext->getDBuilder()->createBasicType("double", 64, llvm::dwarf::DW_ATE_float);
    return dblTy;
}

void ast::DebugInfo::emitLocation(ast::ExprAST *ast) {
    if (!ast) {
        return llvmContext->getBuilder()->SetCurrentDebugLocation(llvm::DebugLoc{});
    }
    llvm::DIScope *scope;
    if (lexicalBlocks.empty()) {
        scope = theCU;
    } else {
        scope = lexicalBlocks.back();
    }
    llvmContext->getBuilder()->SetCurrentDebugLocation(llvm::DILocation::get(
            scope->getContext(), ast->getLine(), ast->getCol(), scope));
}