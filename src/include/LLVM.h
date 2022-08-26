#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <memory>
#include <iostream>
#include "../include/KaleidoscopeJIT.h"

class LLVMContext {
    std::unique_ptr<llvm::LLVMContext> theContext;
    std::unique_ptr<llvm::Module> theModule;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::legacy::FunctionPassManager> theFPM;
    std::unique_ptr<llvm::orc::KaleidoscopeJIT> theJit;
    std::map<char, int> binOpPrecedence;
    llvm::ExitOnError exitOnError;
public:
    LLVMContext() {
        binOpPrecedence = std::map<char, int>{};
        binOpPrecedence['<'] = 10;
        binOpPrecedence['+'] = 20;
        binOpPrecedence['-'] = 20;
        binOpPrecedence['*'] = 40;
        theJit = exitOnError(llvm::orc::KaleidoscopeJIT::Create());
        initializeModuleAndPassManager();
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

    inline const std::unique_ptr<llvm::orc::KaleidoscopeJIT>& getJit() const {
        return theJit;
    }

    inline const std::map<char, int>& getBinOpPrecedence() const {
        return binOpPrecedence;
    }

    void addToBinOpPrecedence(std::pair<char, int> binOp) {
        binOpPrecedence[binOp.first] = binOp.second;
    }

    void initializeModuleAndPassManager() {
        theContext = std::make_unique<llvm::LLVMContext>();
        theModule = std::make_unique<llvm::Module>("my cool jit", *theContext);
        theModule->setDataLayout(theJit->getDataLayout());

        builder = std::make_unique<llvm::IRBuilder<>>(*theContext);

        theFPM = std::make_unique<llvm::legacy::FunctionPassManager>(theModule.get());

        theFPM->add(llvm::createInstructionCombiningPass());
        theFPM->add(llvm::createReassociatePass());
        theFPM->add(llvm::createGVNPass());
        theFPM->add(llvm::createCFGSimplificationPass());

        theFPM->doInitialization();
    }

    void handleTopLevelExprJit() {
        auto rt = theJit->getMainJITDylib().createResourceTracker();
        auto tsm = llvm::orc::ThreadSafeModule{std::move(theModule), std::move(theContext)};
        exitOnError(theJit->addModule(std::move(tsm), rt));

        initializeModuleAndPassManager();

        auto exprSymbol = exitOnError(theJit->lookup("__anon_expr"));

        double (*fp)() = (double (*)())(intptr_t)exprSymbol.getAddress();
        std::cout << "Evaluated to " << fp() << std::endl;

        exitOnError(rt->remove());
    }

    void handleDefinition() {
        auto tsm = llvm::orc::ThreadSafeModule{std::move(theModule), std::move(theContext)};
        exitOnError(theJit->addModule(std::move(tsm)));
        initializeModuleAndPassManager();
    }
};
