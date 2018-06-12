#include <iostream>
#include <algorithm>
#include "Compiler.h"
#include "./sourceScanner/parser.h"
#include "./sourceScanner/lexer.h"
#include "prettyprinter.h"


using namespace std;

// template<class Type1, class Type2>
// bool isInstanceOf(Type1* Instance) {
//     return (dynamic_cast<Type2*>(Instance) != NULL);
// }

Statement* Compiler::parseSourceAST(FILE* infile) {
    void*  scanner;
    yylex_init(&scanner);
    yyset_in(infile, scanner);
    Statement* output;
    int rvalue = yyparse(scanner, output);
    if (rvalue == 1) {
        cout << "Parsing failed" << endl;
        return NULL;
    }
    return output;
}

Compiler::~Compiler() {
    heapc.clean();
    heapf.clean();
}

Compiler::Compiler(){
    this->main = heapf.allocate<FuncBB>();
    this->funcStack.push(main);
    this->rval = NULL;
    this->first = true;
    this->funcMode = true;
    this->lhsOn = false;
    this->recRead = false;

    this->addOn = true;

    this->callMode = false;

    this->heapI = new InstructionHeap();
}

void adjustHelperFieldOps(FuncBB* F, BasicBlock* currB) {
    for ( Instruction* i : *(currB->instructions) ) {
        // filter for only FIELD load and store instructions
        if ( i->operation == Operation::FieldStore || i->operation == Operation::FieldLoad ) {
            // increment by the size of names
            int s = i->operand0.value();
            i->operand0 = experimental::optional<int32_t>(s + F->names->size());
        }
    }
}

void Compiler::adjustFieldOps(FuncBB* F, BasicBlock* cfg) {
    BasicBlock* currB = cfg;
    while (currB != NULL) {
        if (currB->isBoolB == false && currB->isWhile == false) {
            // is a regular basic block
            adjustHelperFieldOps(F, currB);

        } else if (currB->isBoolB == true ) {
            // treat as IF block
            adjustFieldOps(F, currB->trueNext);
            adjustFieldOps(F, currB->falseNext);
            adjustHelperFieldOps(F, currB);
        } else {
            // treat as WHILE block
            adjustFieldOps(F, currB->whileNext);
            adjustHelperFieldOps(F, currB);
        }
        currB = currB->next;
    }
}

void Compiler::getOutFunc(Function* f, FuncBB* F) {
    BasicBlock* currBB = F->cfg;

    f->isNative = F->isNative;
    f->nativeName = F->nativeName;

    // Add names from CFG function to Function
    for ( string s : *(F->names) ) { f->names_.push_back(s); }
    for ( string s : *(F->record_names) ) { f->names_.push_back(s); }
    adjustFieldOps(F, F->cfg);

    // set param count
    f->parameter_count_ = F->paramcount;
    // Add Constants from CFG function to Function
    for ( Constant* c : *(F->constants) ) { f->constants_.push_back(c); }
    // Add free vars
    for (string s : *(F->free_vars)) { f->free_vars_.push_back(s); }
    // Add local vars
    for (string s : *(F->local_vars)) { f->local_vars_.push_back(s); }
    // Add local vars that are referenced
    for (string s : *(F->local_ref_vars)) { f->local_reference_vars_.push_back(s); }        
    // Add functions
    for (FuncBB* fcB : *(F->childsF)) {
        Function* fc = new Function();
        this->getOutFunc(fc, fcB);
        f->functions_.push_back(fc);
    }
    // Add instructions
    this->addInsToFunction(F->cfg, f);

}

void Compiler::addInsToFunction(BasicBlock* bb, Function* f) {
    BasicBlock* currBB = bb;
    while (currBB != NULL) {
        if ( currBB->isBoolB || currBB->isWhile ) {
            if (currBB->isBoolB) {
                // is an IF statement
                for (Instruction* i : *(currBB->instructions) ) { f->instructions.push_back(*i); }
                f->instructions.push_back(*(heapI->allocate<Instruction>(Operation::If, experimental::optional<int32_t>(2))));
                f->instructions.push_back(*(heapI->allocate<Instruction>(Operation::Goto, experimental::optional<int32_t>(0))));
                int pre = f->instructions.size();
                addInsToFunction(currBB->trueNext, f);
                int post = f->instructions.size();
                f->instructions[pre-1].operand0 = experimental::optional<int32_t>(post-pre+2); // skip all the instrutions + second goto
                
                f->instructions.push_back(*(heapI->allocate<Instruction>(Operation::Goto, experimental::optional<int32_t>(0))));
                pre = f->instructions.size();
                addInsToFunction(currBB->falseNext, f);
                post = f->instructions.size();
                f->instructions[pre-1].operand0 = experimental::optional<int32_t>(post-pre+1); // skip all false instructions
            } else {
                // is a while loop
                int prepre = f->instructions.size(); // get the index of while condition evaluation
                for (Instruction* i : *(currBB->instructions) ) { f->instructions.push_back(*i); }
                f->instructions.push_back(*(heapI->allocate<Instruction>(Operation::If, experimental::optional<int32_t>(2))));
                f->instructions.push_back(*(heapI->allocate<Instruction>(Operation::Goto, experimental::optional<int32_t>(0))));
                int pre = f->instructions.size();
                addInsToFunction(currBB->whileNext, f);
                int post = f->instructions.size();
                f->instructions[pre-1].operand0 = experimental::optional<int32_t>(post-pre+2);
                f->instructions.push_back(*(heapI->allocate<Instruction>(Operation::Goto, experimental::optional<int32_t>(prepre-post))));
            }
        } else {
            for (Instruction* i : *(currBB->instructions) ) { f->instructions.push_back(*i); }
        }
        currBB = currBB->next;
    }
}

void Compiler::assignsTopLevel(Statement* s) {
    if (isInstanceOf<Statement,Assignment>(s)) {
        Assignment* ss = (Assignment*) s;
        if (isInstanceOf<LHS,Name>(ss->lhs)) {
            string x = ((Name*) ss->lhs)->name;
            if (find(this->main->names->begin(), this->main->names->end(), x) != this->main->names->end()) {
                // do nothing
            } else {
                this->main->names->push_back( x );
            }
        }
    }

    if (isInstanceOf<Statement,StatementList>(s)) {
        StatementList* ss = (StatementList*) s;
        for (Statement* sss : *(ss->statementList)) {
            assignsTopLevel(sss);
        }
    }

    if (isInstanceOf<Statement,Block>(s)) {
        assignsTopLevel( ( (Block*) s)->stlst );
    }

    if (isInstanceOf<Statement,IfStatement>(s)) {
        assignsTopLevel( ((IfStatement*) s)->ifPart->stlst );
        assignsTopLevel( ((IfStatement*) s)->elsePart->stlst );
    }

    if (isInstanceOf<Statement,WhileLoop>(s)) {
        assignsTopLevel( ((WhileLoop*) s)->body->stlst );
    }
}

void Compiler::visit(StatementList* stmtList) {
    if (this->first) {
        //native function 1
        NameList* nfArgs = new NameList("x");
        Block* nfBody = (Block*) new SPrint();
        Expression* nf = (Expression*) new FunctionDeclaration(nfArgs, nfBody);
        LHS* nfName = (LHS*) new Name("print");
        Assignment* nfAssn = new Assignment(nf, nfName);
        this->assignsTopLevel(nfAssn);
        nfAssn->accept(*this);
        funcStack.top()->latestB->join(rval);


        // native funciton 2
        NameList* nfArgs1 = new NameList("x");
        Block* nfBody1 = (Block*) new SICast();
        Expression* nf1 = (Expression*) new FunctionDeclaration(nfArgs1, nfBody1);
        LHS* nfName1 = (LHS*) new Name("intcast");
        Assignment* nfAssn1 = new Assignment(nf1, nfName1);
        this->assignsTopLevel(nfAssn1);
        nfAssn1->accept(*this);
        funcStack.top()->latestB->join(rval);


        // native funtion 3
        NameList* nfArgs2 = new NameList("x");
        Block* nfBody2 = (Block*) new SInput();
        Expression* nf2 = (Expression*) new FunctionDeclaration(nfArgs2, nfBody2);
        LHS* nfName2 = (LHS*) new Name("input");
        Assignment* nfAssn2 = new Assignment(nf2, nfName2);
        this->assignsTopLevel(nfAssn2);
        nfAssn2->accept(*this);
        funcStack.top()->latestB->join(rval);


        this->first = false;
        for (Statement* s : *(stmtList->statementList) ) {
            this->assignsTopLevel(s);
        }

        delete nfAssn; delete nfAssn1; delete nfAssn2; 
    }

    if (stmtList->statementList->size() == 0 || stmtList == NULL) {
        if (this->funcMode) {
            this->funcMode = false;
        }
        rval = heapc.allocate<BasicBlock>();
        return;
    }


    if (this->funcMode) {
        this->funcMode = false;
        for (Statement* s : *(stmtList->statementList) ) {
            rval = NULL;
            s->accept(*this);
            if (rval != NULL) {
                bool cond1 = rval->isBoolB == false && funcStack.top()->latestB->isBoolB == false;
                bool cond2 = rval->isWhile == false && funcStack.top()->latestB->isWhile == false;
                if ( cond1 && cond2 ) {
                    funcStack.top()->latestB->join(rval);
                } else {
                    funcStack.top()->latestB->next = rval;
                    funcStack.top()->latestB = rval;
                }
            }
        }

    } else {
        BasicBlock* bb = heapc.allocate<BasicBlock>();
        BasicBlock* bbFront = bb;
        for (Statement* s : *(stmtList->statementList) ) {
            rval = NULL;
            s->accept(*this);
            if (rval != NULL) {
                bool cond1 = rval->isBoolB == false && bbFront->isBoolB == false;
                bool cond2 = rval->isWhile == false && bbFront->isWhile == false;
                if ( cond1 && cond2 ) {
                    bbFront->join(rval);
                } else {
                    bbFront->next = rval;
                    bbFront = rval;
                }
            }
        }
        rval = bb;
    }
}

void Compiler::visit(Global* global) {
    // generates no code as global statement is a no op
}

void Compiler::visit(Block* block) {
    block->stlst->accept(*this);
}

void Compiler::visit(IfStatement* ifStatement) {
    ifStatement->condition->accept(*this);
    BasicBlock* condBB = rval;
    ifStatement->ifPart->accept(*this);
    BasicBlock* trueBB = rval;
    ifStatement->elsePart->accept(*this);
    BasicBlock* falseBB = rval;
    condBB->isBoolB = true;
    condBB->trueNext = trueBB;
    condBB->falseNext = falseBB;
    rval = condBB;
}

void Compiler::visit(WhileLoop* whileLoop) {
    whileLoop->condition->accept(*this);
    BasicBlock* condBB = rval;
    whileLoop->body->accept(*this);
    BasicBlock* bodyBB = rval;
    condBB->isWhile = true;
    condBB->whileNext = bodyBB;
    rval = condBB;
}

void adjustHelper(FuncBB* F, BasicBlock* currB) {
    PrettyPrinter p;

    for ( Instruction* i : *(currB->instructions) ) {
        // filter for only push reference instructions
        //p.print(*i, cout); cout << endl;

        if ( i->operation == Operation::PushReference ) {
            // filter for only puch FREE VAR instructions
            if ( i->operand0.value() >= F->local_ref_vars->size() ) {
                //cout << "yep";
                int s = i->operand0.value();
                i->operand0 = experimental::optional<int32_t>(s + 1);
            }
        }
    }
}

void Compiler::adjustPushRefs(FuncBB* F, BasicBlock* cfg) {
    BasicBlock* currB = cfg;
    while (currB != NULL) {
        if (currB->isBoolB == false && currB->isWhile == false) {
            // is a regular basic block
            adjustHelper(F, currB);

        } else if (currB->isBoolB == true ) {
            // treat as IF block
            adjustPushRefs(F, currB->trueNext);
            adjustPushRefs(F, currB->falseNext);
            adjustHelper(F, currB);
        } else {
            // treat as WHILE block
            adjustPushRefs(F, currB->whileNext);
            adjustHelper(F, currB);
        }
        currB = currB->next;
    }
}

void Compiler::referenceLoader(FuncBB* F, string x) {
    if (F == NULL) { throw UninitializedVariableException(x); }

    ptrdiff_t s;
    s = find(F->local_vars->begin(), F->local_vars->end(), x) - F->local_vars->begin();
    if ( s < F->local_vars->size() ) {
        // found in local vars
        // need to increment free_var pushes
        if (callMode) {
            adjustPushRefs(F, F->cfg);

            FuncBB* t = funcStack.top();
            adjustPushRefs(F, t->scanCFG);
            adjustPushRefs(F, t->parentF->scanCFG);
            
        } else {
            adjustPushRefs(F, F->cfg); 
        }

        ptrdiff_t ss = find(F->local_ref_vars->begin(), F->local_ref_vars->end(), x) - F->local_ref_vars->begin();
        if ( ss < F->local_ref_vars->size() ) {
            // found in local ref vars
            BasicBlock* b = heapc.allocate<BasicBlock>(heapI->allocate<Instruction>(Operation::PushReference, experimental::optional<int32_t>(ss)));
            if (addOn) {
                F->refPoint->join(b);
            } else {
                F->latestB->next = b;
                F->latestB = b;
            }
            return;
        }

        int sss = F->local_ref_vars->size();
        F->local_ref_vars->push_back(x);
        BasicBlock* b = heapc.allocate<BasicBlock>(heapI->allocate<Instruction>(Operation::PushReference, experimental::optional<int32_t>(sss)));
        if (addOn) {
            F->refPoint->join(b);
        } else {
            F->latestB->next = b;
            F->latestB = b;
        }
        return;
    }

    // didnt find in local vars so try to find in free vars
    s = find(F->free_vars->begin(), F->free_vars->end(), x) - F->free_vars->begin();
    if ( s < F->free_vars->size() ) {
        // found in free vars
        int ss = s + F->local_ref_vars->size();
        BasicBlock* b = heapc.allocate<BasicBlock>(heapI->allocate<Instruction>(Operation::PushReference, experimental::optional<int32_t>(ss)));
        if (addOn) {
            F->refPoint->join(b);
        } else {
            F->latestB->next = b;
            F->latestB = b;
        }
        return;
    }

    // didnt find in local vars nor in free vars so we add it to free vars push reference then recurse up
    int ss = F->local_ref_vars->size() + F->free_vars->size();
    F->free_vars->push_back(x);
    BasicBlock* b = heapc.allocate<BasicBlock>(heapI->allocate<Instruction>(Operation::PushReference, experimental::optional<int32_t>(ss)));
    if (addOn) {
        F->refPoint->join(b);
    } else {
        F->latestB->next = b;
        F->latestB = b;
    }
    referenceLoader(F->parentF, x);

}

void Compiler::visit(Name* n) {
    string x = n->name;
    FuncBB* currF = this->funcStack.top();
    ptrdiff_t s;

    s = find(currF->names->begin(), currF->names->end(), x) - currF->names->begin();
    if ( s < currF->names->size() ) {
        //found var in globals
        Instruction* i = heapI->allocate<Instruction>(Operation::LoadGlobal, experimental::optional<int32_t>(s));
        rval = heapc.allocate<BasicBlock>(i);
        return;
    }

    s = find(currF->local_vars->begin(), currF->local_vars->end(), x) - currF->local_vars->begin();
    if ( s < currF->local_vars->size() ) {
        //found in local vars
        Instruction* i = heapI->allocate<Instruction>(Operation::LoadLocal, experimental::optional<int32_t>(s));
        rval = heapc.allocate<BasicBlock>(i);
        return;
    }

    s = find(currF->free_vars->begin(), currF->free_vars->end(), x) - currF->free_vars->begin();
    if ( s < currF->free_vars->size() ) {
        //found in free vars
        int ss = s + currF->local_ref_vars->size();
        Instruction* i = heapI->allocate<Instruction>(Operation::PushReference, experimental::optional<int32_t>(s));
        BasicBlock* bb = heapc.allocate<BasicBlock>(i);
        bb->instructions->push_back( heapI->allocate<Instruction>(Operation::LoadReference, experimental::optional<int32_t>()) );
        rval = bb;
        return;
    }

    bool foundInNonGlobalFrame = false;
    if (currF == main) { throw UninitializedVariableException(x); }

    FuncBB* temp = currF->parentF;

    while ( temp != this->main ) {
        ptrdiff_t idx = find(temp->local_vars->begin(), temp->local_vars->end(), x) - temp->local_vars->begin();
        if (idx < temp->local_vars->size() ) {
            foundInNonGlobalFrame = true;
            break;
        }
        temp = temp->parentF;
    }

    if (foundInNonGlobalFrame) {
        int ss = currF->free_vars->size() + currF->local_ref_vars->size();
        currF->free_vars->push_back(x);
        this->referenceLoader(currF->parentF, x);
        Instruction* i = heapI->allocate<Instruction>(Operation::PushReference, experimental::optional<int32_t>(ss));
        rval = heapc.allocate<BasicBlock>(i);
        rval->instructions->push_back(heapI->allocate<Instruction>(Operation::LoadReference, experimental::optional<int32_t>()));
        return;
    } else {
        s = find(this->main->names->begin(), this->main->names->end(), x) - this->main->names->begin();
        if ( s < this->main->names->size() ) {
        // found in global scope
        int ss = currF->names->size();
        currF->names->push_back(x);
        Instruction* i = heapI->allocate<Instruction>(Operation::LoadGlobal, experimental::optional<int32_t>(ss));
        rval = heapc.allocate<BasicBlock>(i);
        return;
        } else {
            throw UninitializedVariableException(x);
        }
    }
}

void Compiler::visit(NoneConst* none) {
    Instruction* i = heapI->allocate<Instruction>(Operation::LoadConst, experimental::optional<int32_t>(0));
    rval = heapc.allocate<BasicBlock>(i);
}

void Compiler::visit(BoolConst* b) {
    int s = 0;
    Constant* booll = (Constant*) new Boolean(b->b);

    for (Constant* c : *(funcStack.top()->constants)) {
        if ( booll->toString() == c->toString() ) {break;} //for some reason bool eq isnt working
        s++;
    }
    if ( s < funcStack.top()->constants->size() ) {
        delete booll;
        Instruction* i = heapI->allocate<Instruction>(Operation::LoadConst, experimental::optional<int32_t>(s));
        rval = heapc.allocate<BasicBlock>(i);
        return;
    }
    s = funcStack.top()->constants->size();
    funcStack.top()->constants->push_back(booll);
    Instruction* i = heapI->allocate<Instruction>(Operation::LoadConst, experimental::optional<int32_t>(s));
    rval = heapc.allocate<BasicBlock>(i);
}

void Compiler::visit(StrConst* ss) {
    int s = 0;
    Constant* str = (Constant*) new String(ss->str);
    for ( Constant* c : *(funcStack.top()->constants) ) {
         if (str->equals(c)) {break;}
         s++;
    }
    if ( s < funcStack.top()->constants->size() ) {
        delete str;
        Instruction* i = heapI->allocate<Instruction>(Operation::LoadConst, experimental::optional<int32_t>(s));
        rval = heapc.allocate<BasicBlock>(i);
        return;
    }

    s = funcStack.top()->constants->size();
    funcStack.top()->constants->push_back(str);
    Instruction* i = heapI->allocate<Instruction>(Operation::LoadConst, experimental::optional<int32_t>(s));
    rval = heapc.allocate<BasicBlock>(i);
}

void Compiler::visit(IntConst* ii) {
    int s = 0;
    Integer* intt = new Integer(ii->i);
    for ( Constant* c : *(funcStack.top()->constants) ) {
        if ( intt->equals(c) ) {break;}
        s++;
    }
    if ( s < funcStack.top()->constants->size() ) {
        delete intt;
        Instruction* i = heapI->allocate<Instruction>(Operation::LoadConst, experimental::optional<int32_t>(s));
        rval = heapc.allocate<BasicBlock>(i);
        return;
    }
    s = funcStack.top()->constants->size();
    Constant* integer = (Constant*) intt;
    funcStack.top()->constants->push_back(integer);
    Instruction* i = heapI->allocate<Instruction>(Operation::LoadConst, experimental::optional<int32_t>(s));
    rval = heapc.allocate<BasicBlock>(i);
}

void Compiler::visit(BinaryExpression* exp) {
    exp->left->accept(*this);
    BasicBlock* leftB = rval;
    exp->right->accept(*this);
    BasicBlock* rightB = rval;

    BasicBlock* ret = heapc.allocate<BasicBlock>();
    ret->join(leftB);
    ret->join(rightB);
    Instruction* i;
    
    if (exp->op == "<") {
        ret->instructions->push_back(heapI->allocate<Instruction>(Operation::Swap, std::experimental::optional<int32_t>()));
        i = heapI->allocate<Instruction>(Operation::Gt, std::experimental::optional<int32_t>() );
    } else if (exp->op == "<=") {
        ret->instructions->push_back(heapI->allocate<Instruction>(Operation::Swap, std::experimental::optional<int32_t>()));
        i = heapI->allocate<Instruction>(Operation::Geq, std::experimental::optional<int32_t>() );
    } else if (exp->op == ">") {
        i = heapI->allocate<Instruction>(Operation::Gt, std::experimental::optional<int32_t>() );
    } else if (exp->op == ">=") {
        i = heapI->allocate<Instruction>(Operation::Geq, std::experimental::optional<int32_t>() );
    } else if (exp->op == "==") {
       i = heapI->allocate<Instruction>(Operation::Eq, std::experimental::optional<int32_t>() );
    } else if (exp->op == "+") {
       i = heapI->allocate<Instruction>(Operation::Add, std::experimental::optional<int32_t>() );
    } else if (exp->op == "-") {
        i = heapI->allocate<Instruction>(Operation::Sub, std::experimental::optional<int32_t>() );
    } else if (exp->op == "/") {
        i = heapI->allocate<Instruction>(Operation::Div, std::experimental::optional<int32_t>() );
    } else if (exp->op == "*") {
        i = heapI->allocate<Instruction>(Operation::Mul, std::experimental::optional<int32_t>() );
    } else if (exp->op == "|") {
        i = heapI->allocate<Instruction>(Operation::Or, std::experimental::optional<int32_t>() );
    } else if (exp->op == "&") {
        i = heapI->allocate<Instruction>(Operation::And, std::experimental::optional<int32_t>() );
    } else {
        throw CompilerError("in binary expression");
    }
    ret->instructions->push_back(i);

    rval = ret;
}

void Compiler::visit(UnaryExpression* exp) {
    exp->ex->accept(*this);
    BasicBlock* exB = rval;

    if (exp->op == "!") {
        exB->instructions->push_back(heapI->allocate<Instruction>(Operation::Not, std::experimental::optional<int32_t>()));
    } else if (exp->op == "-") {
        exB->instructions->push_back(heapI->allocate<Instruction>(Operation::Neg, std::experimental::optional<int32_t>()));
    } else {
        throw CompilerError("in UnaryExpression");
    }
}

/////////////////////////////////////////////////////////////////////////////////////

void Compiler::visit(Return* returnType) {
    returnType->e->accept(*this);
    rval->instructions->push_back(heapI->allocate<Instruction>( Operation::Return, experimental::optional<int32_t>() ));
}

void Compiler::visit(CallStatement* call) {
    call->call->accept(*this);
    if (rval->isBoolB || rval->isWhile) {
        throw CompilerError("fatal: callstatement BasicBlock corrupt");
    } else {    
        rval->instructions->push_back( heapI->allocate<Instruction>( Operation::Pop, experimental::optional<int32_t>() ) );
    } 
}

void Compiler::assignsFuncLevel(Statement* s, FuncBB* f) {
    if (isInstanceOf<Statement,SPrint>(s) || isInstanceOf<Statement,SICast>(s)) {
        return;
    }

    if (isInstanceOf<Statement,Assignment>(s)) {
        Assignment* ss = (Assignment*) s;
        if (isInstanceOf<LHS,Name>(ss->lhs)) {
            string x = ((Name*) ss->lhs)->name;
            if (std::find(f->names->begin(), f->names->end(), x) == f->names->end() ) {
                // only instantiate assigned variables that are not global
                if (find(f->local_vars->begin(),f->local_vars->end(),x) == f->local_vars->end()) {
                    f->local_vars->push_back( x );
                }
            }
        }
    }

    if (isInstanceOf<Statement,StatementList>(s)) {
        StatementList* ss = (StatementList*) s;
        for (Statement* sss : *(ss->statementList)) {
            assignsFuncLevel(sss, f);
        }
    }

    if (isInstanceOf<Statement,Block>(s)) {
        assignsFuncLevel( ( (Block*) s)->stlst, f );
    }

    if (isInstanceOf<Statement,IfStatement>(s)) {
        assignsFuncLevel( ((IfStatement*) s)->ifPart->stlst, f );
        assignsFuncLevel( ((IfStatement*) s)->elsePart->stlst, f );
    }

    if (isInstanceOf<Statement,WhileLoop>(s)) {
        assignsFuncLevel( ((WhileLoop*) s)->body->stlst, f );
    }

}

void Compiler::globals(Statement* s, FuncBB* f) {

    if (isInstanceOf<Statement,SPrint>(s) || isInstanceOf<Statement,SICast>(s)) {
        return;
    }

    if (isInstanceOf<Statement,Global>(s)) {
        Global* ss = (Global*) s;
        f->names->push_back( ss->varname );

        if (find(main->names->begin(), main->names->end(), ss->varname) == main->names->end() ) {
            main->names->push_back(ss->varname);
        }

    }

    if (isInstanceOf<Statement,StatementList>(s)) {
        StatementList* ss = (StatementList*) s;
        for (Statement* sss : *(ss->statementList)) {
            globals(sss, f);
        }
    }

    if (isInstanceOf<Statement,Block>(s)) {
        globals( ( (Block*) s)->stlst, f );
    }

    if (isInstanceOf<Statement,IfStatement>(s)) {
        globals( ((IfStatement*) s)->ifPart->stlst, f );
        globals( ((IfStatement*) s)->elsePart->stlst, f );
    }

    if (isInstanceOf<Statement,WhileLoop>(s)) {
        globals( ((WhileLoop*) s)->body->stlst, f );
    }

}

void Compiler::visit(FunctionDeclaration* funcDec) {
    FuncBB* f = heapf.allocate<FuncBB>();
    f->parentF = this->funcStack.top();

    f->paramcount = funcDec->args->list.size();
    for ( string arg : funcDec->args->list ) {
        f->local_vars->push_back(arg);
    }

    this->globals(funcDec->body, f); // must call globals before assigns
    this->assignsFuncLevel(funcDec->body, f);

    int s = this->funcStack.top()->childsF->size();
    this->funcStack.top()->childsF->push_back(f);
    Instruction* i = heapI->allocate<Instruction>(Operation::LoadFunc, experimental::optional<int32_t>(s));
    BasicBlock* bb = heapc.allocate<BasicBlock>(i);

    this->funcStack.top()->refPoint = bb; // this is the point that references are pushed to

    this->funcStack.push(f);
    this->funcMode = true;
    funcDec->body->accept(*this); // should also push local ref vars onto the stack
    this->funcStack.pop();

    if ( f->latestB->instructions->size() == 0 || (*(f->latestB->instructions))[(f->latestB->instructions->size()-1)]->operation != Operation::Return ) {
        BasicBlock* bb1 = heapc.allocate<BasicBlock>(heapI->allocate<Instruction>(Operation::LoadConst, experimental::optional<int32_t>(0)));
        BasicBlock* bb2 = heapc.allocate<BasicBlock>(heapI->allocate<Instruction>(Operation::Return, experimental::optional<int32_t>()));
        bb1->join(bb2);
        f->latestB->next = bb1;
        f->latestB = bb1;
    }

    int z = f->free_vars->size();
    Instruction* ii = heapI->allocate<Instruction>(Operation::AllocClosure, experimental::optional<int32_t>(z));
    bb->instructions->push_back(ii);
    rval = bb;
}

void Compiler::visit(Assignment* assignment) {
    if ( isInstanceOf<LHS,Name>(assignment->lhs) ) {
        // we are doing a var assignment
        assignment->expression->accept(*this);
        string x = ( (Name*) assignment->lhs )->name;

        if ( funcStack.top() == this->main ) {
            int s = 0;
            for (string xp : *(funcStack.top()->names) ) {
                if (x == xp) {
                    break;
                }
                s++;
                if (s == funcStack.top()->names->size()) {
                    throw CompilerError("in assignment");
                }
            }
            Instruction* i = heapI->allocate<Instruction>(Operation::StoreGlobal, experimental::optional<int32_t>(s));
            rval->instructions->push_back(i);
        } else {

            ptrdiff_t s = find(funcStack.top()->names->begin(), funcStack.top()->names->end(), x) - funcStack.top()->names->begin();
            if ( s < funcStack.top()->names->size() ) {
                int ss = (int) s;
                Instruction* i = heapI->allocate<Instruction>(Operation::StoreGlobal, experimental::optional<int32_t>(ss));
                rval->instructions->push_back(i);
            } else {
                s = find(funcStack.top()->local_vars->begin(), funcStack.top()->local_vars->end(), x) - funcStack.top()->local_vars->begin();
                if ( s < funcStack.top()->local_vars->size() ) {
                    int ss = (int) s;
                    Instruction* i = heapI->allocate<Instruction>(Operation::StoreLocal, experimental::optional<int32_t>(ss));
                    rval->instructions->push_back(i);
                } else {
                    throw CompilerError("in assignment operation");
                }
            } 
        }
    }

    if ( isInstanceOf<LHS,FieldDefer>(assignment->lhs)) {
        // we are doing a field deferred assignment
        ( (FieldDefer*) assignment->lhs)->base->accept(*this);
        BasicBlock* lhsB = rval;

        assignment->expression->accept(*this);
        BasicBlock* expB = rval;
        lhsB->join(expB);

        string x = ( (FieldDefer*) assignment->lhs)->field;
        ptrdiff_t s = find(funcStack.top()->record_names->begin(), funcStack.top()->record_names->end(), x) - funcStack.top()->record_names->begin();
        int ss;
        if ( s < funcStack.top()->record_names->size() ) {
            ss = (int) s;
        } else {
            ss = funcStack.top()->record_names->size();
            funcStack.top()->record_names->push_back(x);
        }
        Instruction* i = heapI->allocate<Instruction>(Operation::FieldStore, experimental::optional<int32_t>(ss));
        lhsB->instructions->push_back(i);

        rval = lhsB;
        return;
    }

    if ( isInstanceOf<LHS, IndexedExpr>(assignment->lhs)) {
        // we are doing an indexed assignment
        ( (IndexedExpr*) assignment->lhs )->base->accept(*this);
        BasicBlock* rec = rval;

        ( (IndexedExpr*) assignment->lhs )->index->accept(*this);
        BasicBlock* val = rval;

        assignment->expression->accept(*this);
        BasicBlock* expr = rval;

        rec->join(val);
        rec->join(expr);

        Instruction* i = heapI->allocate<Instruction>(Operation::IndexStore, experimental::optional<int32_t>());
        rec->instructions->push_back(i);

        rval = rec;

        return;
    }
}

void Compiler::visit(Call* fc) {

    fc->lhs->accept(*this);
    BasicBlock* nameB = rval;
    int end = fc->exl->exList->size() - 1;

    funcStack.top()->scanCFG = nameB;

    for (int j = 0; j <= end; j++) {
        Expression* ex = (*(fc->exl->exList))[j];
        callMode = true;
        ex->accept(*this);
        callMode = false;
        nameB->join(rval);
        
    }

    int s = fc->exl->exList->size();
    Instruction* i = heapI->allocate<Instruction>(Operation::Call, experimental::optional<int32_t>(s));
    nameB->join( heapc.allocate<BasicBlock>(i) );
    rval = nameB;    
}

void Compiler::visit(SICast* sic) { 
    this->funcStack.top()->isNative = true;
    this->funcStack.top()->nativeName = "intcast()";
}

void Compiler::visit(SPrint* spr) {
    this->funcStack.top()->isNative = true;
    this->funcStack.top()->nativeName = "print()";
}

void Compiler::visit(SInput* s) {
    this->funcStack.top()->isNative = true;
    this->funcStack.top()->nativeName = "input()";
}

void Compiler::visit(Record* record) {
    BasicBlock* recBB = heapc.allocate<BasicBlock>(heapI->allocate<Instruction>(Operation::AllocRecord, experimental::optional<int32_t>()));

    for (int i = 0; i < record->m1.size(); i++) {
        // store the values
        recBB->instructions->push_back( heapI->allocate<Instruction>(Operation::Dup, experimental::optional<int32_t>()));

        string xI = record->m1.at(i);
        Expression* yI = record->m2.at(i);

        this->recRead = true;
        yI->accept(*this);
        this->recRead = false;

        recBB->join(rval);

        ptrdiff_t s = find(funcStack.top()->record_names->begin(), funcStack.top()->record_names->end(), xI) - funcStack.top()->record_names->begin();
        int ss;

        if (s < funcStack.top()->record_names->size()) {
            ss = (int) s;

        } else {
            ss = funcStack.top()->record_names->size();
            funcStack.top()->record_names->push_back(xI);
        }

        BasicBlock* b = heapc.allocate<BasicBlock>(heapI->allocate<Instruction>(Operation::FieldStore, experimental::optional<int32_t>(ss)));
        recBB->join(b);
    }
    rval = recBB;
    return;
}

void Compiler::visit(FieldDefer* fde) {
    fde->base->accept(*this);
    BasicBlock* baseB = rval;
    string x = fde->field;
    
    // fix this to support adding stuff
    ptrdiff_t s = find(funcStack.top()->record_names->begin(), funcStack.top()->record_names->end(), x) - funcStack.top()->record_names->begin();
    int ss;
    if ( s >= funcStack.top()->record_names->size() ) {
        ss = funcStack.top()->record_names->size();
        funcStack.top()->record_names->push_back(x);
    } else {
        ss = (int) s;
    }
    Instruction* i = heapI->allocate<Instruction>(Operation::FieldLoad, experimental::optional<int32_t>(ss));
    baseB->instructions->push_back(i);
    rval = baseB;

}

void Compiler::visit(IndexedExpr* ie) {
    ie->base->accept(*this);
    BasicBlock* baseB = rval;

    ie->index->accept(*this);
    BasicBlock* indexB = rval;

    baseB->join(indexB);

    Instruction* i = heapI->allocate<Instruction>(Operation::IndexLoad, experimental::optional<int32_t>());
    baseB->instructions->push_back(i);

    rval = baseB;
}


////////////////////////////////////////////////////////////////////////////////////

void Compiler::visit(LHS* lhs) { throw CompilerError("ABSTRACT"); }
void Compiler::visit(Expression* expression) { throw CompilerError("ABSTRACT"); }
void Compiler::visit(Statement* statement) { throw CompilerError("ABSTRACT"); }
void Compiler::visit(ExpressionList* exprLst) { throw CompilerError("ExList"); }
void Compiler::visit(NameList* nl) { throw CompilerError("NameList"); }

