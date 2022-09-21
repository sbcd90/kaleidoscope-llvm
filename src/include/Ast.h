#include <utility>
#include <vector>
#include <map>
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include "LLVM.h"
#include "Debugger.h"

namespace ast {
    static std::map<std::string, llvm::AllocaInst*> namedValues;

    llvm::raw_ostream& indent(llvm::raw_ostream &o, int size) {
        return o << std::string(size, ' ');
    }

    class ExprAST {
        SourceLocation loc;
    public:
        ExprAST(SourceLocation loc = curLoc): loc(loc) {}
        virtual ~ExprAST() = default;
        virtual llvm::Value* codegen() = 0;
        int getLine() const {
            return loc.line;
        }
        int getCol() const {
            return loc.col;
        }
        virtual llvm::raw_ostream& dump(llvm::raw_ostream &out, int ind) {
            return out << ':' << getLine() << ':' << getCol()  << '\n';
        }
    };

    class  NumberExprAST: public ExprAST {
        double val;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        NumberExprAST(double val, std::shared_ptr<LLVMContext> llvmContext): val(val), llvmContext(std::move(llvmContext)) {}

        llvm::Value* codegen() override;

        llvm::raw_ostream& dump(llvm::raw_ostream &out, int ind) override {
            return ast::ExprAST::dump(out << val, ind);
        }
    };

    class VariableExprAST: public ExprAST {
        std::string name;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        VariableExprAST(SourceLocation loc, std::string name, std::shared_ptr<LLVMContext> llvmContext): ast::ExprAST(loc), name(std::move(name)), llvmContext(std::move(llvmContext)) {}

        llvm::Value* codegen() override;

        inline const std::string& getName() const {
            return name;
        }

        llvm::raw_ostream& dump(llvm::raw_ostream &out, int ind) override {
            return ast::ExprAST::dump(out << name, ind);
        }
    };

    class UnaryExprAST: public ExprAST {
        char op;
        std::unique_ptr<ExprAST> operand;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        UnaryExprAST(char op, std::unique_ptr<ExprAST> operand, std::shared_ptr<LLVMContext> llvmContext):
            op(op), operand(std::move(operand)), llvmContext(std::move(llvmContext)) {}

        llvm::Value* codegen() override;

        llvm::raw_ostream& dump(llvm::raw_ostream &out, int ind) override {
            ast::ExprAST::dump(out << "unary" << op, ind);
            operand->dump(out, ind + 1);
            return out;
        }
    };

    class BinaryExprAST: public ExprAST {
        char op;
        std::unique_ptr<ExprAST> lhs;
        std::unique_ptr<ExprAST> rhs;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        BinaryExprAST(SourceLocation loc, char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs, std::shared_ptr<LLVMContext> llvmContext):
            ast::ExprAST(loc), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)), llvmContext(std::move(llvmContext)) {}

        llvm::Value* codegen() override;

        llvm::raw_ostream& dump(llvm::raw_ostream &out, int ind) override {
            ast::ExprAST::dump(out << "binary" << op, ind);
            lhs->dump(indent(out, ind) << "LHS:", ind + 1);
            rhs->dump(indent(out, ind) << "RHS:", ind + 1);
            return out;
        }
    };

    class CallexprAST: public ExprAST {
        std::string callee;
        std::vector<std::unique_ptr<ExprAST>> args;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        CallexprAST(SourceLocation loc, std::string callee, std::vector<std::unique_ptr<ExprAST>> args, std::shared_ptr<LLVMContext> llvmContext):
            ast::ExprAST(loc), callee(std::move(callee)), args(std::move(args)), llvmContext(std::move(llvmContext)) {}

        llvm::Value* codegen() override;

        llvm::raw_ostream& dump(llvm::raw_ostream &out, int ind) override {
            ast::ExprAST::dump(out << "call" << callee, ind);
            for (const auto &arg: args) {
                arg->dump(indent(out, ind + 1), ind + 1);
            }
            return out;
        }
    };

    class IfExprAST: public ExprAST {
        std::unique_ptr<ExprAST> cond, then, else_;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        IfExprAST(SourceLocation loc, std::unique_ptr<ExprAST> cond, std::unique_ptr<ExprAST> then, std::unique_ptr<ExprAST> else_, std::shared_ptr<LLVMContext> llvmContext):
        ast::ExprAST(loc), cond(std::move(cond)), then(std::move(then)), else_(std::move(else_)), llvmContext(std::move(llvmContext)) {}

        llvm::Value* codegen() override;

        llvm::raw_ostream& dump(llvm::raw_ostream &out, int ind) override {
            ast::ExprAST::dump(out << "if", ind);
            cond->dump(indent(out, ind) << "Cond:", ind + 1);
            then->dump(indent(out, ind) << "Then:", ind + 1);
            else_->dump(indent(out, ind) << "Else:", ind + 1);
            return out;
        }
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

        llvm::raw_ostream& dump(llvm::raw_ostream &out, int ind) override {
            ast::ExprAST::dump(out << "for", ind);
            start->dump(indent(out, ind) << "Cond:", ind + 1);
            end->dump(indent(out, ind) << "End:", ind + 1);
            step->dump(indent(out, ind) << "Step:", ind + 1);
            body->dump(indent(out, ind) << "Body:", ind + 1);
            return out;
        }
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
        int line;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        PrototypeAST(SourceLocation loc, std::string name, std::vector<std::string> args, std::shared_ptr<LLVMContext> llvmContext,
                     bool isOperator = false, unsigned prec = 0):
            name(std::move(name)), args(std::move(args)), llvmContext(std::move(llvmContext)),
            isOperator(isOperator), precedence(prec), line(loc.line) {}

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

        int getLine() const {
            return line;
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

        llvm::raw_ostream& dump(llvm::raw_ostream &out, int ind) {
            indent(out, ind) << "FunctionAST\n";
            ++ind;
            indent(out, ind) << "Body:";
            return body ? body->dump(out, ind): out << "null\n";
        }
    };

    class VarExprAST : public ExprAST {
        std::vector<std::pair<std::string, std::unique_ptr<ast::ExprAST>>> varNames;
        std::unique_ptr<ast::ExprAST> body;
        std::shared_ptr<LLVMContext> llvmContext;
    public:
        VarExprAST(std::vector<std::pair<std::string, std::unique_ptr<ast::ExprAST>>> varNames,
                   std::unique_ptr<ast::ExprAST> body, std::shared_ptr<LLVMContext> llvmContext):
                   varNames(std::move(varNames)), body(std::move(body)), llvmContext(std::move(llvmContext)) {}

        llvm::Value* codegen() override;

        llvm::raw_ostream &dump(llvm::raw_ostream &out, int ind) override {
            ast::ExprAST::dump(out << "var", ind);
            for (const auto &namedVar: varNames) {
                namedVar.second->dump(indent(out, ind) << namedVar.first << ':', ind + 1);
            }
            body->dump(indent(out, ind) << "Body:", ind + 1);
            return out;
        }
    };
}