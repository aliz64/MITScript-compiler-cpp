#pragma once
#include <iostream>
#include "./sourceScanner/Visitor.h" //incldues AST.h
#include "types.h"
#include "CFG.h"
#include <stack>
#include "../gc/gc.h"

using namespace std;

class Compiler : public Visitor {
public:
    Compiler();

    cfgHeap heapc;
    FuncBBheap heapf;
    InstructionHeap* heapI;

    FuncBB* main;
    BasicBlock* rval;
    std::stack<FuncBB*> funcStack;
    //std::stack<BasicBlock*> bbStack;
    bool first;
    bool funcMode;
    bool lhsOn;
    bool recRead;

    bool addOn;

    bool callMode;

    virtual ~Compiler();

    void getOutFunc(Function* f, FuncBB* F);
    void addInsToFunction(BasicBlock* bb, Function* f);
    void assignsTopLevel(Statement* s);
    void assignsFuncLevel(Statement* s, FuncBB* f);
    void globals(Statement* s, FuncBB* f);

    void adjustPushRefs(FuncBB* f, BasicBlock* cfg);
    void referenceLoader(FuncBB* F, string x);

    void adjustFieldOps(FuncBB* F, BasicBlock* cfg);

    Statement* parseSourceAST(FILE* infile);

    void visit(SICast* sic);
    void visit(SInput* s);
    void visit(SPrint* spr);
    void visit(Assignment* assignment);
    void visit(StatementList* stmtList);
    void visit(Global* global);
    void visit(Statement* statement);
    void visit(Block* block);
    void visit(IfStatement* ifStatement);
    void visit(WhileLoop* whileLoop);
    void visit(Return* returnType);
    void visit(CallStatement* call);
    void visit(Expression* expression);
    void visit(BinaryExpression* exp);
    void visit(UnaryExpression* exp);
    void visit(Record* record);
    void visit(FunctionDeclaration* funcDec);
    void visit(FieldDefer* fde);
    void visit(IndexedExpr* ie);
    void visit(LHS* lhs);
    void visit(ExpressionList* exprLst);
    void visit(NameList* nl);
    void visit(Call* c);
    void visit(Name* n);
    void visit(NoneConst* none);
    void visit(BoolConst* b);
    void visit(StrConst* s);
    void visit(IntConst* i);
};

class CompilerException {
public:
    string msg;
    CompilerException() : msg("") {}
    CompilerException(string msg) : msg(msg) {}
    virtual string message() {
        return msg;
    }
};

class CompilerError : public CompilerException {
public:
    CompilerError() : CompilerException("CompilerError") { }
    CompilerError(string x) : CompilerException("CompilerError: "+x) {}
};

class UninitializedVariableException : public CompilerException {
public:
    UninitializedVariableException() : CompilerException("UninitializedVariableException") {}
    UninitializedVariableException(string m) : CompilerException("UninitializedVariableException: "+m) {}

};

/*class StaticAnalyzer : public Visitor {

    void visit(SICast* sic);
    void visit(SPrint* spr);
    void visit(Assignment* assignment);
    void visit(StatementList* stmtList);
    void visit(Global* global);
    void visit(Statement* statement);
    void visit(Block* block);
    void visit(IfStatement* ifStatement);
    void visit(WhileLoop* whileLoop);
    void visit(Return* returnType);
    void visit(CallStatement* call);
    void visit(Expression* expression);
    void visit(BinaryExpression* exp);
    void visit(UnaryExpression* exp);
    void visit(Record* record);
    void visit(FunctionDeclaration* funcDec);
    void visit(FieldDefer* fde);
    void visit(IndexedExpr* ie);
    void visit(LHS* lhs);
    void visit(ExpressionList* exprLst);
    void visit(NameList* nl);
    void visit(Call* c);
    void visit(Name* n);
    void visit(NoneConst* none);
    void visit(BoolConst* b);
    void visit(StrConst* s);
    void visit(IntConst* i);

};*/

































