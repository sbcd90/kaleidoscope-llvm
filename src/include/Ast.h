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

    class IfExprAST: public ExprAST {
        std::unique_ptr<ExprAST> cond, then, else_;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        IfExprAST(std::unique_ptr<ExprAST> cond, std::unique_ptr<ExprAST> then, std::unique_ptr<ExprAST> else_, std::shared_ptr<LLVMContext> llvmContext):
        cond(std::move(cond)), then(std::move(then)), else_(std::move(else_)), llvmContext(std::move(llvmContext)) {}

        llvm::Value* codegen() override;
    };

    class ForExprAST: public ExprAST {
        std::string varName;
        std::unique_ptr<ExprAST> start, end, step, body;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        ForExprAST(std::string varName, std::unique_ptr<ExprAST> start,
                   std::unique_ptr<ExprAST> end, std::unique_ptr<ExprAST> step,
                   std::unique_ptr<ExprAST> body, std::shared_ptr<LLVMContext> llvmContext): varName(std::move(varName)), start(std::move(start)),
                   end(std::move(end)), step(std::move(step)), body(std::move(body)), llvmContext(std::move(llvmContext)) {}

        llvm::Value* codegen() override;
    };

    class WhileExprAST: public ExprAST {
        std::unique_ptr<ExprAST> end, body;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        WhileExprAST(std::unique_ptr<ExprAST> end, std::unique_ptr<ExprAST> body, std::shared_ptr<LLVMContext> llvmContext):
            end(std::move(end)), body(std::move(body)), llvmContext(std::move(llvmContext)) {}

        llvm::Value* codegen() override;
    };

    class PrototypeAST {
        std::string name;
        std::vector<std::string> args;
        bool isOperator;
        unsigned precedence;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        PrototypeAST(std::string name, std::vector<std::string> args, std::shared_ptr<LLVMContext> llvmContext,
                     bool isOperator = false, unsigned prec = 0):
            name(std::move(name)), args(std::move(args)), llvmContext(std::move(llvmContext)),
            isOperator(isOperator), precedence(prec) {}

        llvm::Function* codegen();

        const std::string& getName() const {
            return this->name;
        }

        bool isUnaryOp() const {
            return isOperator && args.size() == 1;
        }

        bool isBinaryOp() const {
            return isOperator && args.size() == 2;
        }

        char getOperatorName() const {
            assert(isUnaryOp() || isBinaryOp());
            return name[name.length() - 1];
        }

        unsigned getBinaryPrecedence() const {
            return precedence;
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