/*!
*/
#include "Interpreter.h"
#include <string.h>

using namespace std;

template<class Type1, class Type2>
bool isInstanceOf(Type1* Instance) {
    return (dynamic_cast<Type2*>(Instance) != NULL);
}

Interpreter::Interpreter() {
    rval = NULL;
    globalReturn = new unordered_set<string>();
    StackFrame* globalFrame = new StackFrame();
    globalFrame->gFrame = globalFrame;
    gamma.push(globalFrame);

    // add global functions to this frame
    const char* c = "toPrint";
    NameList* nl = new NameList(c);
    SPrint* p = new SPrint();
    Function* print = new Function(globalFrame, nl, p);
    globalFrame->frame->insert({"print",print});

    const char* c2 = "toCast";
    NameList* nl2 = new NameList(c2);
    SICast* p2 = new SICast();
    Function* atoif = new Function(globalFrame, nl2, p2);
    globalFrame->frame->insert({"intcast",atoif});

}

void Interpreter::visit(SICast* s) {
    Value* input = gamma.top()->frame->find("toCast")->second;
    if ( !isInstanceOf<Value,String>(input) ) {
        throw IllegalCastException();
    }
    String* inputS = (String*) input;
    Integer* out = new Integer(atoi(inputS->_s.c_str()));
    ReturnVal* r = new ReturnVal(out);
    rval = r;
}

void Interpreter::visit(SPrint* p) {
    cout << gamma.top()->frame->find("toPrint")->second->toString() << endl;
}

void Interpreter::visit(Assignment* assignment) {

    if ( isInstanceOf<LHS, Name>(assignment->lhs) ) {
        // we are doing a Var Assignment
        // eval the RHS
        assignment->expression->accept(*this);

        // check whether to update local or global frame 
        StackFrame* a1 = gamma.top();
        StackFrame* a2;
        string x = ( (Name*) assignment->lhs )->name;
        if ( a1->globalVars->find(x) == a1->globalVars->end() ) {
            a2 = a1;
        } else {
            a2 = a1->gFrame;
        }
        //update/create the address of Value in the frame
        (*(a2->frame))[x] = rval;
    }

    if ( isInstanceOf<LHS, FieldDefer>(assignment->lhs) ) {
        // we are doing a field assignment
        ( (FieldDefer*) assignment->lhs )->base->accept(*this);
        if ( !isInstanceOf<Value,RecordVal>(rval) ) {
            throw IllegalCastException();
        }
        // get the base expression (record) and field x
        RecordVal* e1 = (RecordVal*) rval;
        string x = ( (FieldDefer*) assignment->lhs )->field;
        // eval the RHS
        assignment->expression->accept(*this);
        if ( e1->_record->find(x) == e1->_record->end() ) {
            e1->add(x, rval);
        } else {
            e1->_record->find(x)->second = rval;
        }
    }

    if ( isInstanceOf<LHS, IndexedExpr>(assignment->lhs) ) {
        // we are doing a heap indexed assignment
        ( (IndexedExpr*) assignment->lhs )->base->accept(*this);
        if ( !isInstanceOf<Value,RecordVal>(rval) ) {
            throw IllegalCastException();
        }
        RecordVal* r = (RecordVal*) rval;
        ( (IndexedExpr*) assignment->lhs )->index->accept(*this);
        string x = rval->toString();
        assignment->expression->accept(*this);
        if ( r->_record->find(x) == r->_record->end() ) {
            r->add(x, rval);
        } else {
            r->_record->find(x)->second = rval;
        }
    }
}

void Interpreter::visit(StatementList* stmtList) {
    if (stmtList->statementList->size() == 0) {
        return; // all statements executed
    }
    rval = NULL;
    for (Statement* s : *(stmtList->statementList)) {
        s->accept(*this);
        if ( isInstanceOf<Value, ReturnVal>(rval) ) {
        break;
        }
    }
}

void Interpreter::visit(Global* global) {
    // dont have to do anything?
    // or do we just add the variable to the current global set?
}

void Interpreter::visit(Block* block) {
    block->stlst->accept(*this);
}

void Interpreter::visit(IfStatement* ifStatement) {
    ifStatement->condition->accept(*this);
    if ( isInstanceOf<Value, Bool>(rval) ) {
        Bool* rvalB = (Bool*) rval;
        if ( rvalB->_b == true ) {
            ifStatement->ifPart->accept(*this);
        } else {
            ifStatement->elsePart->accept(*this);
        }
    } else {
        throw IllegalCastException();
    }
}

void Interpreter::visit(WhileLoop* whileLoop) {
    whileLoop->condition->accept(*this);
    if ( isInstanceOf<Value, Bool>(rval) ) {
        Bool* rvalB = (Bool*) rval;
        if (rvalB->_b == true) {
            whileLoop->body->accept(*this);
            whileLoop->accept(*this);
        }
    } else {
        throw IllegalCastException();
    }
}

void Interpreter::visit(Return* returnType) {
    returnType->e->accept(*this);
    ReturnVal* r = new ReturnVal(rval);
    rval = r;
}

void Interpreter::visit(UnaryExpression* exp) {
    exp->ex->accept(*this);
    if (exp->op == "!") {
        if ( isInstanceOf<Value, Bool>(rval) ) {
            Bool* val = (Bool*) rval;
            bool n = (val->_b ? false : true);
            rval = new Bool(n);
        } else {
            throw IllegalCastException();
        }
    }

    if (exp->op == "-") {
        if ( isInstanceOf<Value, Integer>(rval) ) {
            Integer* val = (Integer*) rval;
            rval = new Integer( -1 * val->_n );
        } else {
            throw IllegalCastException();
        }
    }
}

void Interpreter::visit(class BinaryExpression* exp) {
    exp->left->accept(*this);
    Value* leftVal = rval;
    exp->right->accept(*this);
    Value* rightVal = rval;

    if ( exp->op == "<" || exp->op == "<=" || exp->op == ">" || exp->op == ">=") {
        if ( !isInstanceOf<Value,Integer>(leftVal) || !isInstanceOf<Value,Integer>(rightVal) ) {
            throw IllegalCastException();
        }
        Integer* leftValInt = (Integer*) leftVal;
        Integer* rightValInt = (Integer*) rightVal; bool z;
        if      ( exp->op == "<" ) { z = (leftValInt->_n <  rightValInt->_n); }
        else if ( exp->op == "<=") { z = (leftValInt->_n <= rightValInt->_n); }
        else if ( exp->op == ">" ) { z = (leftValInt->_n >  rightValInt->_n); }
        else if ( exp->op == ">=") { z = (leftValInt->_n >= rightValInt->_n); }
        else { throw 1; }
        rval = new Bool(z);
    }

    if (exp->op == "|" || exp->op == "&" ) {
        if ( !isInstanceOf<Value,Bool>(leftVal) || !isInstanceOf<Value,Bool>(rightVal) ) {
            throw IllegalCastException();
        }
        Bool* lb = (Bool*) leftVal;
        Bool* rb = (Bool*) rightVal; bool z;
        if ( exp->op == "|" ) { z = lb->_b || rb->_b; }
        if ( exp->op == "&" ) { z = lb->_b && rb->_b; }
        rval = new Bool(z);
    }

    if ( exp->op == "-" || exp->op == "*" || exp->op == "/" ) {
        if ( !isInstanceOf<Value,Integer>(leftVal) || !isInstanceOf<Value,Integer>(rightVal) ) {
            throw IllegalCastException();
        }
        Integer* leftValInt = (Integer*) leftVal;
        Integer* rightValInt = (Integer*) rightVal; int z;
        if (exp->op == "-") {z = leftValInt->_n - rightValInt->_n; }
        if (exp->op == "*") {z = leftValInt->_n * rightValInt->_n; }
        if (exp->op == "/") {
            if (rightValInt->_n == 0) { throw IllegalArithmeticException(); }
            z = leftValInt->_n / rightValInt->_n;
        }
        rval = new Integer(z);
    }

    if ( exp->op == "+") {
        if ( isInstanceOf<Value,Integer>(leftVal) && isInstanceOf<Value,Integer>(rightVal) ) {
            Integer* leftValInt = (Integer*) leftVal;
            Integer* rightValInt = (Integer*) rightVal; int z;
            z = leftValInt->_n + rightValInt->_n;
            rval = new Integer(z);
        } else if ( isInstanceOf<Value,String>(leftVal) || isInstanceOf<Value,String>(rightVal) ) {
            stringstream ss1, ss2;
            ss1 << leftVal->toString();
            ss2 << rightVal->toString();
            string s1 = ss1.str(); //s1 = s1.substr( 1, s1.length()-2 );
            string s2 = ss2.str(); //s2 = s2.substr( 1, s2.length()-2 );
            rval = new String( s1+s2 );
        } else {
            throw IllegalCastException();
        }
    }

    if (exp->op == "==") {
        if ( isInstanceOf<Value,None>(leftVal) && isInstanceOf<Value,None>(rightVal) ) {
            rval = new Bool(true);
            return;
        }
        if ( isInstanceOf<Value,Integer>(leftVal) && isInstanceOf<Value,Integer>(rightVal) ) {
            Integer* leftValInt = (Integer*) leftVal;
            Integer* rightValInt = (Integer*) rightVal;
            bool z = (leftValInt->_n == rightValInt->_n);
            rval = new Bool(z);
            return;
        }
        if ( isInstanceOf<Value,Bool>(leftVal) && isInstanceOf<Value,Bool>(rightVal) ) {
            Bool* leftValInt = (Bool*) leftVal;
            Bool* rightValInt = (Bool*) rightVal;
            bool z = (leftValInt->_b == rightValInt->_b);
            rval = new Bool(z);
            return;
        }
        if ( isInstanceOf<Value,String>(leftVal) && isInstanceOf<Value,String>(rightVal) ) {
            String* leftValInt = (String*) leftVal;
            String* rightValInt = (String*) rightVal;
            bool z = (leftValInt->_s == rightValInt->_s);
            rval = new Bool(z);
            return;
        }
        if ( isInstanceOf<Value,RecordVal>(leftVal) && isInstanceOf<Value,RecordVal>(rightVal) ) {
            RecordVal* leftValInt = (RecordVal*) leftVal;
            RecordVal* rightValInt = (RecordVal*) rightVal;
            bool z = (leftValInt == rightValInt); // checking for pointer equality
            rval = new Bool(z);
            return;
        }
        if ( isInstanceOf<Value,Function>(leftVal) && isInstanceOf<Value,Function>(rightVal) ) {
            Function* leftValInt = (Function*) leftVal;
            Function* rightValInt = (Function*) rightVal;
            bool z = false;
            if ( leftValInt->frame == rightValInt->frame ) {
                if ( leftValInt->body == rightValInt->body ) {
                    if ( leftValInt->args->list.size() == rightValInt->args->list.size() ) {
                        z = true; // how to check function equality?
                    }
                }
            }
            rval = new Bool(z);
            return;
        }
        rval = new Bool(false);

    }
}

Value* lookup_read(StackFrame* a, string& x) {
    if (a->globalVars->find(x) != a->globalVars->end()) {
        return (a->gFrame->frame->find(x)->second);
    }

    // local/scoped
    if ( a->frame->find(x) == a->frame->end() ) {
        if (a->parent == NULL) {
            throw UninitializedVariableException(x);
        }
        return lookup_read(a->parent, x);
    } else {
        return a->frame->find(x)->second;
    }
}

void Interpreter::visit(Name* n) {
    StackFrame* a1 = gamma.top();
    string x = n->name;
    Value* y = lookup_read(a1, x);
    rval = y;
}

void Interpreter::visit(NoneConst* none) {
    rval = new None();
}

void Interpreter::visit(BoolConst* b) {
    rval = new Bool( b->b );
}

void Interpreter::visit(StrConst* s) {
    rval = new String( s->str );
}

void Interpreter::visit(IntConst* i) {
    rval = new Integer( i->i );
}

void Interpreter::visit(Record* record) {
    RecordVal* r  = new RecordVal();
    for (int i = 0; i < record->m1.size(); ++i) {
        string xi = record->m1.at(i);
        record->m2.at(i)->accept(*this);
        Value* ai = rval;
        r->add(xi, ai);
    }
    rval = r;
}

void Interpreter::visit(FunctionDeclaration* funcDec) {
    Function* f = new Function( gamma.top(), funcDec->args, funcDec->body );
    rval = f;
}


void Interpreter::visit(FieldDefer* fde) {
    fde->base->accept(*this);
    if ( !isInstanceOf<Value, RecordVal>(rval) ) {
        throw IllegalCastException();
    }
    RecordVal* r = (RecordVal*) rval;
    string x = fde->field;
    if ( r->_record->find(x) != r->_record->end() ) {
        rval = r->_record->find(x)->second;
    } else {
        rval = new None();
    }
}

void Interpreter::visit(IndexedExpr* ie) {
    ie->base->accept(*this);
    if ( !isInstanceOf<Value, RecordVal>(rval) ) {
        throw IllegalCastException();
    }
    RecordVal* r = (RecordVal*) rval;
    ie->index->accept(*this);
    string x = rval->toString();
    if ( r->_record->find(x) != r->_record->end() ) {
        rval = r->_record->find(x)->second;
    } else {
        rval = new None();
    }
}

void Interpreter::visit(CallStatement* call) {
    call->call->accept(*this);
}


void Interpreter::visit(ExpressionList* exprLst) {
    throw 1;
}

void Interpreter::visit(NameList* nl) {
    throw 1;
}

void Interpreter::visit(Call* fc) {
    fc->lhs->accept(*this);
    if ( !isInstanceOf<Value,Function>(rval) ) {
        throw IllegalCastException();
    }
    Function* f = (Function*) rval;

    StackFrame* sig = new StackFrame();
    sig->parent = f->frame;
    sig->gFrame = gamma.top()->gFrame;

    // add global vars to frame
    globalReturn = new unordered_set<string>;
    this->globals(f->body);
    for (string x : *globalReturn) {
        sig->globalVars->insert(x);
    }

    // initialize to none in frame for assigns
    globalReturn = new unordered_set<string>;
    this->assigns(f->body);
    for ( string x : *globalReturn ) {
        sig->frame->insert( { x, new None() } );
    }

    // initialize agrs
    if (f->args->list.size() != fc->exl->exList->size() ) {
        throw RuntimeException();
    }
    for (int i = 0; i < fc->exl->exList->size(); i++) {
        string arg = f->args->list.at(i);
        fc->exl->exList->at(i)->accept(*this);
        (*(sig->frame))[arg] = rval;
    }

    gamma.push(sig);
    f->body->accept(*this);
    gamma.pop();

    if ( isInstanceOf<Value,ReturnVal>(rval) ) {
        ReturnVal* x = (ReturnVal*) rval;
        Value* r = x->val;
        rval = r;
    } else {
        None* n = new None();
        rval = n;
    }

}

void Interpreter::globals(Statement* s) {

    if ( isInstanceOf<Statement,Global>(s) ) {
        Global* ss = (Global*) s;
        globalReturn->insert(ss->varname);
    }

    if ( isInstanceOf<Statement,StatementList>(s) ) {
        StatementList* ss = (StatementList*) s;
        if (ss->statementList->size() == 0) {
        } else {
            for ( Statement* sss : *(ss->statementList) ) {
                globals(sss);
            }
        }
    }

    if ( isInstanceOf<Statement,Block>(s) ) {
        Block* ss = (Block*) s;
        globals(ss->stlst);
    }

    if ( isInstanceOf<Statement, IfStatement>(s) ) {
        StatementList* s1 = ((IfStatement*) s)->ifPart->stlst;
        StatementList* s2 = ((IfStatement*) s)->elsePart->stlst;
        globals(s1); globals(s2);
    }

    if ( isInstanceOf<Statement, WhileLoop>(s) ) {
        StatementList* s1 = ((WhileLoop*) s)->body->stlst;
        globals(s1);
    }
}

void Interpreter::assigns(Statement* s) {

    if ( isInstanceOf<Statement,Assignment>(s) ) {
        Assignment* ss = (Assignment*) s;
        if ( isInstanceOf<LHS, Name>(ss->lhs) ) {
            globalReturn->insert( ((Name*) ss->lhs)->name );
        }
    }

    if ( isInstanceOf<Statement,StatementList>(s) ) {
        StatementList* ss = (StatementList*) s;
        if (ss->statementList->size() == 0) {
        } else {
            for ( Statement* sss : *(ss->statementList) ) {
                assigns(sss);
            }
        }
    }

    if ( isInstanceOf<Statement,Block>(s) ) {
        Block* ss = (Block*) s;
        assigns(ss->stlst);
    }

    if ( isInstanceOf<Statement, IfStatement>(s) ) {
        StatementList* s1 = ((IfStatement*) s)->ifPart->stlst;
        StatementList* s2 = ((IfStatement*) s)->elsePart->stlst;
        assigns(s1); assigns(s2);
    }

    if ( isInstanceOf<Statement, WhileLoop>(s) ) {
        StatementList* s1 = ((WhileLoop*) s)->body->stlst;
        assigns(s1);
    }
}

// Abstract classes should never be visited
void Interpreter::visit(LHS* lhs) { throw 1; }
void Interpreter::visit(Statement* statement) { throw 1; }
void Interpreter::visit(Expression* expression) { throw 1; }
