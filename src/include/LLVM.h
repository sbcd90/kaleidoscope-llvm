#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <memory>

class LLVMContext {
    std::unique_ptr<llvm::LLVMContext> theContext;
    std::unique_ptr<llvm::Module> theModule;
    std::unique_ptr<llvm::IRBuilder<>> builder;
public:
    LLVMContext() {
        theContext = std::make_unique<llvm::LLVMContext>();
        theModule = std::make_unique<llvm::Module>("my cool jit", *theContext);

        builder = std::make_unique<llvm::IRBuilder<>>(*theContext);
    }

    inline const std::unique_ptr<llvm::LLVMContext>& getContext() const {
        return theContext;
    }

    inline const std::unique_ptr<llvm::Module>& getModule() const {
        return theModule;
    }

    inline const std::unique_ptr<llvm::IRBuilder<>>& getBuilder() const {
        return builder;
    }
};
