#include "llvm/IR/DIBuilder.h"
#include "Ast.h"
#include <vector>


static llvm::DISubroutineType* CreateFunctionType(unsigned numArgs, const std::shared_ptr<LLVMContext> &llvmContext, const std::shared_ptr<ast::DebugInfo> &ksDebugInfo) {
    llvm::SmallVector<llvm::Metadata*, 8> eltTys{};

    auto dblTy = ksDebugInfo->getDoubleTy();

    eltTys.push_back(dblTy);
    for (unsigned i = 0, e = numArgs; i != e; ++i) {
        eltTys.push_back(dblTy);
    }
    return llvmContext->getDBuilder()->createSubroutineType(llvmContext->getDBuilder()->getOrCreateTypeArray(eltTys));
}