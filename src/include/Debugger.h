#include "llvm/IR/DIBuilder.h"
#include "Ast.h"
#include <vector>

struct DebugInfo {
    llvm::DICompileUnit *theCU;
    llvm::DIType *dblTy;
    std::vector<llvm::DIScope*> lexicalBlocks;

    void emitLocation(ast::ExprAST *ast);
    llvm::DIType *getDoubleTy();
} KSDbgInfo;

struct SourceLocation {
    int line;
    int col;
};

static SourceLocation curLoc;
static SourceLocation lexLoc{1, 0};

static int advance() {
    auto lastChar = getchar();

    if (lastChar == '\n' || lastChar == '\r') {
        ++lexLoc.line;
        lexLoc.col = 0;
    } else {
        ++lexLoc.col;
    }
    return lastChar;
}

static std::unique_ptr<llvm::DIBuilder> dBuilder;