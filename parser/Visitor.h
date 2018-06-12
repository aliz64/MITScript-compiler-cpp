#pragma once

#include <memory>
#include "AST.h"

using namespace std;

class Visitor {
public:
    virtual void visit(SICast* s) = 0;
    virtual void visit(SPrint* p) = 0;
    virtual void visit(StatementList* stmtList) = 0;
    virtual void visit(Global* global) = 0;
    virtual void visit(Statement* statement) = 0;
    virtual void visit(Block* block) = 0;
    virtual void visit(IfStatement* ifStatement) = 0;
    virtual void visit(WhileLoop* whileLoop) = 0;
    virtual void visit(Assignment* assignment) = 0;
    virtual void visit(Return* returnType) = 0;
    virtual void visit(CallStatement* call) = 0;
    virtual void visit(Expression* expression) = 0;
    virtual void visit(BinaryExpression* exp) = 0;
    virtual void visit(UnaryExpression* exp) = 0;
    virtual void visit(Record* record) = 0;
    virtual void visit(FunctionDeclaration* funcDec) = 0;
    virtual void visit(FieldDefer* fde) = 0;
    virtual void visit(IndexedExpr* ie) = 0;
    virtual void visit(LHS* lhs) = 0;
    virtual void visit(ExpressionList* exprLst) = 0;
    virtual void visit(NameList* nl) = 0;
    virtual void visit(Call* c) = 0;
    virtual void visit(Name* n) = 0;
    virtual void visit(NoneConst* none) = 0;
    virtual void visit(BoolConst* b) = 0;
    virtual void visit(StrConst* s) = 0;
    virtual void visit(IntConst* i) = 0;
};
