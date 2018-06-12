#include "Visitor.h"
#include "AST.h"

void StatementList::accept(Visitor& v) {
   v.visit(this);  
}
void Block::accept(Visitor& v) {
    v.visit(this);  
}
void Global::accept(Visitor& v) {
    v.visit(this);  
}
void FieldDefer::accept(Visitor& v) {
    v.visit(this);  
}
void IndexedExpr::accept(Visitor& v) {
    v.visit(this);  
}
void ExpressionList::accept(Visitor& v) {
    v.visit(this);  
}
void BinaryExpression::accept(Visitor& v) {
    v.visit(this);  
}
void UnaryExpression::accept(Visitor& v) {
    v.visit(this);  
}
void NameList::accept(Visitor& v) {
    v.visit(this);  
}
void FunctionDeclaration::accept(Visitor& v) {
    v.visit(this);  
}
void Record::accept(Visitor& v) {
    v.visit(this);  
}
void Assignment::accept(Visitor& v) {
    v.visit(this);  
}
void CallStatement::accept(Visitor& v) {
    v.visit(this);  
}
void Call::accept(Visitor& v) {
    v.visit(this);  
}
void Name::accept(Visitor& v) {
    v.visit(this);  
}
void IfStatement::accept(Visitor& v) {
    v.visit(this);  
}
void WhileLoop::accept(Visitor& v) {
    v.visit(this);  
}
void Return::accept(Visitor& v) {
    v.visit(this);  
}
void IntConst::accept(Visitor& v) {
    v.visit(this);  
}
void StrConst::accept(Visitor& v) {
    v.visit(this);  
}
void BoolConst::accept(Visitor& v) {
    v.visit(this);  
}
void NoneConst::accept(Visitor& v) {
    v.visit(this);  
}

void SPrint::accept(Visitor& v) {
    v.visit(this);
}

void SICast::accept(Visitor& v) {
    v.visit(this);
}

