#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <memory>

static std::unique_ptr<llvm::LLVMContext> theContext;
static std::unique_ptr<llvm::Module> theModule;
static std::unique_ptr<llvm::IRBuilder<>> builder;

static void initializeModule() {
    theContext = std::make_unique<llvm::LLVMContext>();
    theModule = std::make_unique<llvm::Module>("my cool jit", *theContext);

    builder = std::make_unique<llvm::IRBuilder<>>(*theContext);
}
