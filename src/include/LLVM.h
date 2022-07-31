#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <memory>
#include "../include/KaleidoscopeJIT.h"

class LLVMContext {
    std::unique_ptr<llvm::LLVMContext> theContext;
    std::unique_ptr<llvm::Module> theModule;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::legacy::FunctionPassManager> theFPM;
    std::unique_ptr<llvm::orc::KaleidoscopeJIT> theJit;
    llvm::ExitOnError exitOnError;
public:
    LLVMContext() {
        theJit = exitOnError(llvm::orc::KaleidoscopeJIT::Create());
        theContext = std::make_unique<llvm::LLVMContext>();
        theModule = std::make_unique<llvm::Module>("my cool jit", *theContext);

        builder = std::make_unique<llvm::IRBuilder<>>(*theContext);
        initializePassManager();
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

    inline const std::unique_ptr<llvm::legacy::FunctionPassManager>& getFPM() const {
        return theFPM;
    }

    void initializePassManager() {
        theFPM = std::make_unique<llvm::legacy::FunctionPassManager>(theModule.get());

        theFPM->add(llvm::createInstructionCombiningPass());
        theFPM->add(llvm::createReassociatePass());
        theFPM->add(llvm::createGVNPass());
        theFPM->add(llvm::createCFGSimplificationPass());

        theFPM->doInitialization();
    }
};
