#include <utility>
#include <vector>
#include <map>
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include "LLVM.h"

namespace ast {
    static std::map<std::string, llvm::Value*> namedValues;

    class ExprAST {
    public:
        virtual ~ExprAST() = default;
        virtual llvm::Value* codegen() = 0;
    };

    class  NumberExprAST: public ExprAST {
        double val;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        NumberExprAST(double val, std::shared_ptr<LLVMContext> llvmContext): val(val), llvmContext(std::move(llvmContext)) {}

        llvm::Value* codegen() override;
    };

    class VariableExprAST: public ExprAST {
        std::string name;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        VariableExprAST(std::string name, std::shared_ptr<LLVMContext> llvmContext): name(std::move(name)), llvmContext(std::move(llvmContext)) {}

        llvm::Value* codegen() override;
    };

    class BinaryExprAST: public ExprAST {
        char op;
        std::unique_ptr<ExprAST> lhs;
        std::unique_ptr<ExprAST> rhs;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs, std::shared_ptr<LLVMContext> llvmContext):
            op(op), lhs(std::move(lhs)), rhs(std::move(rhs)), llvmContext(std::move(llvmContext)) {}

        llvm::Value* codegen() override;
    };

    class CallexprAST: public ExprAST {
        std::string callee;
        std::vector<std::unique_ptr<ExprAST>> args;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        CallexprAST(std::string callee, std::vector<std::unique_ptr<ExprAST>> args, std::shared_ptr<LLVMContext> llvmContext):
            callee(std::move(callee)), args(std::move(args)), llvmContext(std::move(llvmContext)) {}

        llvm::Value* codegen() override;
    };

    class PrototypeAST {
        std::string name;
        std::vector<std::string> args;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        PrototypeAST(std::string name, std::vector<std::string> args, std::shared_ptr<LLVMContext> llvmContext):
            name(std::move(name)), args(std::move(args)), llvmContext(std::move(llvmContext)) {}

        llvm::Function* codegen();

        const std::string& getName() const {
            return this->name;
        }
    };
    static std::map<std::string, std::unique_ptr<PrototypeAST>> functionProtos;

    static llvm::Function* getFunction(const std::shared_ptr<LLVMContext> &llvmContext, std::string name) {
        if (auto *f = llvmContext->getModule()->getFunction(name)) {
            return f;
        }

        auto fi = functionProtos.find(name);
        if (fi != functionProtos.end()) {
            return fi->second->codegen();
        }

        return nullptr;
    }

    class FunctionAST {
        std::unique_ptr<PrototypeAST> proto;
        std::unique_ptr<ExprAST> body;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body, std::shared_ptr<LLVMContext> llvmContext):
            proto(std::move(proto)), body(std::move(body)), llvmContext(std::move(llvmContext)) {}

        llvm::Function* codegen();
    };
}