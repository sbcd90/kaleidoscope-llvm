#include <llvm/ADT/Optional.h>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <iostream>
#include <system_error>
#include <utility>

class LLVMContext {
    std::unique_ptr<llvm::LLVMContext> theContext;
    std::unique_ptr<llvm::Module> theModule;
    std::unique_ptr<llvm::IRBuilder<>> builder;
//    std::unique_ptr<llvm::legacy::FunctionPassManager> theFPM;
//    std::unique_ptr<llvm::orc::KaleidoscopeJIT> theJit;
    std::map<char, int> binOpPrecedence;
    llvm::ExitOnError exitOnError;
public:
    LLVMContext() {
        binOpPrecedence = std::map<char, int>{};
        binOpPrecedence['='] = 2;
        binOpPrecedence['<'] = 10;
        binOpPrecedence['+'] = 20;
        binOpPrecedence['-'] = 20;
        binOpPrecedence['*'] = 40;
//        theJit = exitOnError(llvm::orc::KaleidoscopeJIT::Create());
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

/*   inline const std::unique_ptr<llvm::legacy::FunctionPassManager>& getFPM() const {
           return theFPM;
       }

       inline const std::unique_ptr<llvm::orc::KaleidoscopeJIT>& getJit() const {
           return theJit;
       }*/

    inline const std::map<char, int>& getBinOpPrecedence() const {
        return binOpPrecedence;
    }

    void addToBinOpPrecedence(std::pair<char, int> binOp) {
        binOpPrecedence[binOp.first] = binOp.second;
    }

    void eraseFromBinOpPrecedence(char binOp) {
        binOpPrecedence.erase(binOp);
    }

    void initializeModuleAndPassManager() {
        theContext = std::make_unique<llvm::LLVMContext>();
        theModule = std::make_unique<llvm::Module>("my cool jit", *theContext);
//        theModule->setDataLayout(theJit->getDataLayout());

        builder = std::make_unique<llvm::IRBuilder<>>(*theContext);

/*        theFPM = std::make_unique<llvm::legacy::FunctionPassManager>(theModule.get());

        theFPM->add(llvm::createInstructionCombiningPass());
        theFPM->add(llvm::createReassociatePass());
        theFPM->add(llvm::createGVNPass());
        theFPM->add(llvm::createCFGSimplificationPass());

        theFPM->doInitialization();*/
    }

    void initializeTargetRegistry() {
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        auto targetTriple = llvm::sys::getDefaultTargetTriple();
        theModule->setTargetTriple(targetTriple);

        std::string error;
        auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

        if (!target) {
            llvm::errs() << error;
            return;
        }

        auto cpu = "generic";
        auto features = "";

        llvm::TargetOptions opt;
        auto rm = llvm::Optional<llvm::Reloc::Model>();
        auto theTargetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, rm);

        theModule->setDataLayout(theTargetMachine->createDataLayout());

        auto filename = "/home/sbcd90/Documents/programs/kaleidoscope-llvm/src/output.o";
        std::error_code ec;
        llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);

        if (ec) {
            llvm::errs() << "Could not open file: " << ec.message();
            return;
        }

        llvm::legacy::PassManager pass;
        auto fileType = llvm::CGFT_ObjectFile;

        if (theTargetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
            llvm::errs() << "TheTargetMachine can't emit a file of this type";
        }

        pass.run(*theModule);
        dest.flush();

        llvm::outs() << "Wrote " << filename << "\n";
    }

/*    void handleTopLevelExprJit() {
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
    }*/

    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function *theFunction, llvm::StringRef varName) {
        llvm::IRBuilder<> tmpB{&theFunction->getEntryBlock(), theFunction->getEntryBlock().begin()};
        return tmpB.CreateAlloca(llvm::Type::getDoubleTy(*theContext), nullptr, varName);
    }
};
