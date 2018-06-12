/*!
This contains the header file for the interpreter
(assignment 2 6.S081 - Ali Zartash)
*/
#pragma once
#include <map>
#include <stack>
#include "Value.h"
#include "Visitor.h"
using namespace std;

class InterpreterException {
public:
    virtual string message() = 0;
};

class UninitializedVariableException : public InterpreterException {
public:
    string msg;
    UninitializedVariableException(string m) : msg(m) {}
    string message() { return "UninitializedVariableException: "+msg; }
};

class IllegalCastException : public InterpreterException {
public:
    string message() { return "IllegalCastException"; }
};

class IllegalArithmeticException : public InterpreterException {
public:
    string message() { return "IllegalArithmeticException: divide by zero"; }
};

class RuntimeException : public InterpreterException {
public:
    string message() { return "RuntimeException"; }
};

class DebugException : public InterpreterException {
public:
    string message() { return "DebugException"; }
};

// The Interpreter class with methods defined in the 
// corresponding .cpp file
// Contains a stack of frames and a return value as state
class Interpreter : public Visitor {
public:
    Value* rval;
    std::stack<StackFrame*> gamma;
    unordered_set<string>* globalReturn;

    void globals(Statement* s);
    void assigns(Statement* s);

    Interpreter();
    void visit(SPrint* p);
    void visit(SICast* s);

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
