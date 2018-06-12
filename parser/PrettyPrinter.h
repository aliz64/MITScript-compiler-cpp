#pragma once

#include <string>
#include <iostream>
#include "Visitor.h"

using namespace std;

class Printer : public std::ostream {
public:
    int indent;
    Printer() : indent(0) { }
};

Printer& operator<<(Printer& p, const char* s) {
    string o = "";
    for (int i = 0; i < p.indent; i++) { o += " "; }
    cout << o << s;
    return p;
}

class PrettyPrinter : public Visitor {
public:

    Printer cout2;

    PrettyPrinter() : cout2() {}

    void visit(StatementList* stmtLst) {      
        for (auto value : *(stmtLst->statementList) ) {
            value->accept(*this);
        }
    }

    void visit(SPrint* p) {}
    void visit(SICast* p) {}
 
    void visit(Global* global) {
        cout << "global " << global->varname << ";" << "\n";
    }

    void visit(Block* block) {
        cout << endl;
        cout2 << "{" << "\n";
        cout2.indent += 4;
        block->stlst->accept(*this);
        cout2.indent -= 4;
        cout2 << "\n" << "}";
    }

    void visit(IfStatement* ifStatement) {
        cout2 << "if (";
        ifStatement->condition->accept(*this);
        cout << ") ";
        ifStatement->ifPart->accept(*this);
        cout << "\n" << "else ";
        ifStatement->elsePart->accept(*this);  
    }

    void visit(WhileLoop* whileLoop) {
        cout2 << "while (";
        whileLoop->condition->accept(*this);
        cout << ") ";
        whileLoop->body->accept(*this);
        cout << "\n";  
    }

    void visit(Return* returnType) {
        cout2 << "return (";
        returnType->e->accept(*this);
        cout << ");" << "\n";    
    }

    void visit(BinaryExpression* exp) {
        cout << "(";
        exp->left->accept(*this);   
        cout << ")";
        cout << exp->op;
        exp->right->accept(*this);
    }

    void visit(UnaryExpression* exp) {
        cout << exp->op;
        cout << "(";
        exp->ex->accept(*this);
        cout << ")";
    }

    void visit(FunctionDeclaration* funcDec) {
        cout << "fun";
        cout << "(";
        funcDec->args->accept(*this);
        cout << ") ";
        funcDec->body->accept(*this);
    }

    void visit(FieldDefer* fde) {
        fde->base->accept(*this);
        cout << "." ;
        cout << fde->field ;    
    }

    void visit(IndexedExpr* ie) {
        ie->base->accept(*this);
        cout << "[";
        ie->index->accept(*this);
        cout << "]";
    }

    void visit(ExpressionList* exprLst) {
        int i = 0;
        for (auto value : *(exprLst->exList)) {
            i++;
            value->accept(*this);
            if (i < (exprLst->exList)->size()-1) {
                cout << ", ";
            }
        }
    }

    void visit(NameList* nl) {        
        int i = 0;
        for (auto value : nl->list) {
            i++;
            cout << value;
            if (i < (nl->list).size()-1) {
                cout << ", ";
            }
        }
    }

    void visit(Record* record) {
        cout << "{ ";
        for (int i = 0; i < record->m1.size(); i++) {
            cout << record->m1.at(i);
            cout << ": ";
            record->m2.at(i)->accept(*this);
            if ( i < record->m2.size() -1 ) {
                cout << "; ";
            }
        }
        cout << " }";
    }   

    void visit(Call* c) {
        c->lhs->accept(*this);
        cout << "(";
        c->exl->accept(*this);
        cout << ")";
    }

    void visit(Name* n) {
        cout << n->name;    
    }
    
    void visit(CallStatement* call) {
        cout2 << "";
        call->call->accept(*this);
        cout << ";" << "\n";    
    
    }
    void visit(Assignment* as) {
        cout2 << "";
        as->lhs->accept(*this);
        cout << " = ";
        as->expression->accept(*this);
        cout << ";" << "\n";
    }

    void visit(NoneConst* none) { cout << "none"; }
    void visit(BoolConst* b) { cout << b->b; }
    void visit(StrConst* s) { cout << s->str; }
    void visit(IntConst* i) { cout << i->i; }

    void visit(Statement* statement) { throw 1; } 
    void visit(LHS* lhs) { throw 1; } 
    void visit(Expression* expression) { throw 1; } 
};
