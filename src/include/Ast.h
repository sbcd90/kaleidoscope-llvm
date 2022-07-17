#include <utility>
#include <vector>
#include <map>
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"

namespace ast {
    static std::map<std::string, llvm::Value*> namedValues;

    class ExprAST {
    public:
        virtual ~ExprAST() = default;
        virtual llvm::Value* codegen() = 0;
    };

    class NumberExprAST: public ExprAST {
        double val;
    public:
        NumberExprAST(double val): val(val) {}

        llvm::Value* codegen() override;
    };

    class VariableExprAST: public ExprAST {
        std::string name;
    public:
        VariableExprAST(std::string name): name(std::move(name)) {}

        llvm::Value* codegen() override;
    };

    class BinaryExprAST: public ExprAST {
        char op;
        std::unique_ptr<ExprAST> lhs;
        std::unique_ptr<ExprAST> rhs;
    public:
        BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs):
            op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

        llvm::Value* codegen() override;
    };

    class CallexprAST: public ExprAST {
        std::string callee;
        std::vector<std::unique_ptr<ExprAST>> args;
    public:
        CallexprAST(std::string callee, std::vector<std::unique_ptr<ExprAST>> args):
            callee(std::move(callee)), args(std::move(args)) {}

        llvm::Value* codegen() override;
    };

    class PrototypeAST: public ExprAST {
        std::string name;
        std::vector<std::string> args;
    public:
        PrototypeAST(std::string name, std::vector<std::string> args):
            name(std::move(name)), args(std::move(args)) {}

        llvm::Function* codegen();

        const std::string& getName() const {
            return this->name;
        }
    };

    class FunctionAST {
        std::unique_ptr<PrototypeAST> proto;
        std::unique_ptr<ExprAST> body;
    public:
        FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body):
            proto(std::move(proto)), body(std::move(body)) {}

        llvm::Function* codegen();
    };
}