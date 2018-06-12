#include "Interpreter.h"
#include <memory>
#include <algorithm>
#include <string.h>

using namespace std;

Interpreter::Interpreter(CollectedHeap& heapp) : heap(heapp) {
    this->rval = NULL;
    this->first = true;
    this->gFrame = NULL;

    this->gcFirst = true;
}

Value* Interpreter::interpret(Function* f) {
    
    // implement NATIVE functions
    if (f->isNative) {
        if (f->nativeName == "print()") {
            cout << (gamma.top()->local_vars)[0]->toString() << endl;
            return heap.allocate<None>();
        }
        if (f->nativeName == "intcast()") {
            Value* v = (gamma.top()->local_vars)[0];
            if (v->t != Type::STRING) {throw IllegalCastException();}
            String* vv = (String*) v;
            int vvv = atoi((vv->value).c_str());
            if (vvv == 0) {
                for (int idx = 0; idx < (vv->value).length(); idx++) {
                    if ( !strchr( "0", (vv->value)[idx] ) ) { throw IllegalCastException(); }
                }
            }
            return heap.allocate<Integer>(vvv);
        } else {
            throw RuntimeException("native function unknown");
        }
        
    }

    // If we are in the MAIN funciton
    if (this->first) {
        this->first = false;
        this->main = f;
        stackF.push(f);
        this->gFrame = heap.allocate<Frame>();
        gamma.push(gFrame);

        // load global constants
        for ( Constant* c : f->constants_ ) {
            Value* v = c->copyTo(heap); // copying constants from C++ to heap
            gFrame->constants.push_back(v);
        }

        // load global name map
        for (string g : (f->names_) ) {
            gFrame->names[g] = heap.allocate<None>();
        }

        // Load NAMES
        for (string g : f->names_ ) {
            gamma.top()->namesIdx.push_back(g);
        }
        
        // initialize FUNCTIONS
        for ( Function* ff : f->functions_ ) {
            Function* f = ff;
            gamma.top()->functions.push_back(f);
        }
    }

    // if not the main function - the top of the stackFrame should already be populated

    PC = 0;
    nextPC = 1;
    while ( PC < f->instructions.size() ) {
        
        garbageCollect();

        execute(f->instructions[PC]);

        if (gamma.top() == gFrame) {
            //opStack.printStack(); cout << endl;
        }

        PC = nextPC;
        nextPC += 1;
        if (rval != NULL) {
            break;
        }
    }

    return rval;
}

void Interpreter::garbageCollect() {
    if ( !heap.full() ) {
        return; // heap is not over allocated
    }

    //opStack.printStack(); cout << endl;

    vector<Collectable*> rootset;

    for (Frame* f : gamma) {
        rootset.push_back(f);
    }

    for (Value* v : *(opStack.stk) )  {
        rootset.push_back(v);
    }

    //cout << heap.count() << " " << heap.usage() << endl;

    heap.gc(rootset.begin(), rootset.end());

    //cout << heap.count() << " " << heap.usage() << endl << endl;

}

void Interpreter::execute(Instruction i) {
    switch (i.operation) {
        case Operation::LoadConst:
            {
            //cout << "LoadConst" << endl;

            int idx = i.operand0.value();
            opStack.push( (gamma.top()->constants)[idx]);
            }
            break;

        case Operation::LoadFunc:
            {
            //cout << "LoadFunc" << endl;

            int idx = i.operand0.value();
            opStack.push( (gamma.top()->functions)[idx] );
            }
            break;

        case Operation::LoadLocal:
            {
            //cout << "LoadLocal" << endl;

            int idx = i.operand0.value();
            opStack.push( (gamma.top()->local_vars)[idx] );
            }
            break;

        case Operation::StoreLocal:
            {
            //cout << "StoreLocal" << endl;

            int idx = i.operand0.value();
            (gamma.top()->local_vars)[idx] = opStack.top();
            opStack.pop();
            }
            break;

        case Operation::LoadGlobal:
            {
            //cout << "LoadGlobal" << endl;

            int idx = i.operand0.value();
            string gN = (gamma.top()->namesIdx)[idx];
            Value* gV = (gFrame->names)[gN];
            opStack.push( gV );
            }
            break;

        case Operation::StoreGlobal:
            {
            //cout << "StoreGlobal" << endl;

            int idx = i.operand0.value();
            string gN = (gamma.top()->namesIdx)[idx];
            (gFrame->names)[gN] = opStack.top();
            opStack.pop();
            }
            break;

        case Operation::PushReference:
            {
            //cout << "PushReference" << endl;

            int idx = i.operand0.value();
            if ( idx < gamma.top()->local_ref_vars.size() ) {
                VarRef* vR = (gamma.top()->local_ref_vars)[idx];
                opStack.push( vR );
            } else {
                idx = idx - gamma.top()->local_ref_vars.size();
                VarRef* vR = (gamma.top()->free_vars)[idx];
                opStack.push( vR );
            }
            }
            break;

        case Operation::LoadReference:
            {
            //cout << "LoadReference" << endl;

            Value* v = opStack.top(); opStack.pop();
            if ( v->t != Type::REFERENCE ) {
                throw RuntimeException("tried to load non reference type");
            } else {
                Value* deref = *(((VarRef*) v)->refValue);
                opStack.push(deref);
            }
            }
            break;

        case Operation::StoreReference:
            {
            //cout << "StoreReference" << endl;
            throw UnimplementedException();
            }
            break;

        case Operation::AllocRecord:
            {
            //cout << "AllocRecord" << endl;
            
            Value* v = (Value*) heap.allocate<RecordVal>();
            opStack.push(v);
            }
            break;

        case Operation::FieldLoad:
            {
            //cout << "FieldLoad" << endl;

            int idx = i.operand0.value();
            Value* op1 = opStack.pop();
            if (op1->t != Type::RECORD) {
                throw IllegalCastException("LINE "+to_string(PC)+" fieldLoad Expected Record, got: "+op1->toString());
            }
            string x = (gamma.top()->namesIdx)[idx];
            Value* z = ((RecordVal*) op1)->getValue(x);
            opStack.push(z);


            // debugging
            //if (idx == 1) {
            //    cout << op1->toString() << endl;
            //    cout << x << endl;
            //    cout << PC << endl;
            //    cout << z->toString() << endl << endl;
            //}

            }
            break;

        case Operation::FieldStore:
            {
            //cout << "FieldStore" << endl;
            
            int idx = i.operand0.value();
            Value* op1 = opStack.pop();
            Value* op2 = opStack.pop();

            if (op2->t != Type::RECORD) {
                throw IllegalCastException("Not a Record");
            }
            string x = (gamma.top()->namesIdx)[idx];
            ((RecordVal*) op2)->addValue(x, op1);

            heap.used += (x.length()*sizeof(char)+sizeof(void*));

            }
            break;

        case Operation::IndexLoad:
            {
            //cout << "IndexLoad" << endl;
            
            Value* op1 = opStack.pop();
            Value* op2 = opStack.pop();
            if (op2->t != Type::RECORD) {
                throw IllegalCastException("Not a Record");
            }

            string x = op1->toString();
            Value* z = ((RecordVal*) op2)->getValue(x);
            opStack.push(z);

            }
            break;

        case Operation::IndexStore:
            {
            //cout << "IndexStore" << endl;
            
            Value* op1 = opStack.pop();
            Value* op2 = opStack.pop();
            Value* op3 = opStack.pop();
            if (op3->t != Type::RECORD) {
                throw IllegalCastException(" index store");
            }
            string x = op2->toString();
            ((RecordVal*) op3)->addValue(x, op1);

            heap.used += (x.length()*sizeof(char)+sizeof(void*));


            }
            break;

        case Operation::AllocClosure:
            {
            //cout << "AllocClosure" << endl;

            int idx = i.operand0.value();
            int refCount = idx;
            Closure* closure = heap.allocate<Closure>();

            // get the references for FREE VARS and put into the closure
            closure->free_vars.resize(refCount);

            for (int j = 0; j < refCount; j++) {
                Value* popped = opStack.top(); opStack.pop();
                if (popped->t != Type::REFERENCE) { throw RuntimeException("line: "+to_string(PC)+"Expected: Reference; Got: "+popped->toString()); }
                VarRef* vR = (VarRef*) popped;
                (closure->free_vars)[refCount-j-1] = vR;
            }

            // put the function in the closure
            Value* fpop = opStack.top(); opStack.pop();
            if (fpop->t != Type::FUNCTION) { throw IllegalCastException("not a function"); }
            closure->func = (Function*) fpop;
            opStack.push(closure);
            }

            break;

        case Operation::Call:
            {
            //cout << "Call" << endl;

            int idx = i.operand0.value();
            Frame* frame = heap.allocate<Frame>();

            // ADD arguments to local vars            
            int argN = idx;
            frame->local_vars.resize(argN);
            for (int j = 0; j < argN; j++) {
                Value* arg = opStack.top(); opStack.pop();
                (frame->local_vars)[argN-j-1] = arg;

                heap.used += sizeof(void*);
            }

            Value* ff = opStack.top(); opStack.pop();
            if (ff->t != Type::CLOSURE) { throw IllegalCastException("not a function"); }
            Closure* f = (Closure*) ff;

            if (f->func->isNative) {
                if (argN != 1) { throw RuntimeException(); }
                gamma.push(frame);
                Value* r = interpret(f->func);
                gamma.pop();
                opStack.push(r);
                return;
            }

            // add free vars to frame
            frame->free_vars = f->free_vars;

            // set parameter count
            frame->parameter_count = f->func->parameter_count_;
            if (argN != frame->parameter_count) { throw RuntimeException(); }

            // add CONSTANTS
            for ( Constant* cptr : f->func->constants_ ) {
                frame->constants.push_back( cptr->copyTo(heap) ); //copy constant form C++ to heap

                heap.used += sizeof(void*);
            }

            // add FUNCTIONS
            for ( Function* fptr : f->func->functions_ ) {
                frame->functions.push_back(fptr);
            }

            // adjust local var length
            for (int j = argN; j < f->func->local_vars_.size(); j++) {
                frame->local_vars.push_back(heap.allocate<None>());

                heap.used += sizeof(void*);

            }

            // add local variable references
            for (string local : f->func->local_reference_vars_ ) {

                ptrdiff_t s;
                s = find(f->func->local_vars_.begin(), f->func->local_vars_.end(), local) - f->func->local_vars_.begin();

                int i = (int) s; // get index in local_vars in i
                
                VarRef* vR = heap.allocate<VarRef>();
                vR->refValue = &((frame->local_vars)[i]);
                frame->local_ref_vars.push_back(vR);

                heap.used += sizeof(void*);
            }

            // set the names used
            frame->namesIdx = f->func->names_;

            // save state
            int pc = PC; int nextpc = nextPC;

            gamma.push(frame);
            Value* r = interpret(f->func);
            gamma.pop();
            
            // restore state
            PC = pc; nextPC = nextpc;
            rval = NULL;

            opStack.push(r);

            }
            break;

        case Operation::Return:
            {
            //cout << "Return" << endl;
            rval = opStack.top(); opStack.pop();
            }
            break;

        case Operation::Sub:
            {
            //cout << "Sub" << endl;
            Value* op1 = opStack.pop();
            Value* op2 = opStack.pop();

            if (op1->t != Type::INTEGER && op2->t != Type::INTEGER) {
                throw IllegalCastException(" sub op");
            }

            Value* result = (Value*) heap.allocate<Integer>( ((Integer*) op2)->value - ((Integer*) op1)->value );
            opStack.push(result);

            }
            break;

        case Operation::Mul:
            {
            //cout << "Mul" << endl;
            Value* op1 = opStack.pop();
            Value* op2 = opStack.pop();

            if (op1->t != Type::INTEGER && op2->t != Type::INTEGER) {
                throw IllegalCastException(" mul op");
            }

            Value* result = (Value*) heap.allocate<Integer>( ((Integer*) op2)->value * ((Integer*) op1)->value );
            opStack.push(result);
            }
            break;

        case Operation::Div:
            {
            //cout << "Div" << endl;
            Value* op1 = opStack.pop();
            Value* op2 = opStack.pop();

            if (op1->t != Type::INTEGER && op2->t != Type::INTEGER) {
                throw IllegalCastException(" op");
            }

            if (((Integer*) op1)->value == 0) { throw IllegalArithmeticException("divide by zero"); }

            Value* result = (Value*) heap.allocate<Integer>( ((Integer*) op2)->value / ((Integer*) op1)->value );
            opStack.push(result);
            }
            break;

        case Operation::Neg:
            {
            //cout << "Neg" << endl;
            Value* op1 = opStack.pop();

            if (op1->t != Type::INTEGER) {
                throw IllegalCastException(" neg ");
            }

            Value* result = (Value*) heap.allocate<Integer>( - ((Integer*) op1)->value );
            opStack.push(result);
            }
            break;

        case Operation::Gt:
            {
            //cout << "Gt" << endl;
            Value* op1 = opStack.pop();
            Value* op2 = opStack.pop();

            if (op1->t != Type::INTEGER && op2->t != Type::INTEGER) {
                throw IllegalCastException(" gt ");
            }

            Value* result = (Value*) heap.allocate<Boolean>( ((Integer*) op2)->value > ((Integer*) op1)->value );
            opStack.push(result);
            }
            break;

        case Operation::Geq:
            {
            //cout << "Geq" << endl;
            Value* op1 = opStack.pop();
            Value* op2 = opStack.pop();

            if (op1->t != Type::INTEGER && op2->t != Type::INTEGER) {
                throw IllegalCastException(" geq ");
            }

            Value* result = (Value*) heap.allocate<Boolean>( ((Integer*) op2)->value >= ((Integer*) op1)->value );
            opStack.push(result);
            }
            break;

        case Operation::And:
            {
            //cout << "And" << endl;
            Value* op1 = opStack.pop();
            Value* op2 = opStack.pop();

            if (op1->t != Type::BOOLEAN && op2->t != Type::BOOLEAN) {
                throw IllegalCastException("  and ");
            }

            Value* result = (Value*) heap.allocate<Boolean>( ((Boolean*) op2)->value && ((Boolean*) op1)->value );
            opStack.push(result);
            }
            break;

        case Operation::Or:
            {
            //cout << "Or" << endl;
            Value* op1 = opStack.pop();
            Value* op2 = opStack.pop();

            if (op1->t != Type::BOOLEAN && op2->t != Type::BOOLEAN) {
                throw IllegalCastException("or ");
            }

            Value* result = (Value*) heap.allocate<Boolean>( ((Boolean*) op2)->value || ((Boolean*) op1)->value );
            opStack.push(result);
            }
            break;

        case Operation::Not:
            {
            //cout << "Not" << endl;
            Value* op1 = opStack.pop();

            if (op1->t != Type::BOOLEAN) {
                throw IllegalCastException(" not");
            }

            Value* result = (Value*) heap.allocate<Boolean>( ! ((Boolean*) op1)->value );
            opStack.push(result);
            }
            break;

        case Operation::Add:
            {
            //cout << "Add" << endl;
            Value* op1 = opStack.pop();
            Value* op2 = opStack.pop();
            Value* result;

            if (op1->t == Type::INTEGER && op2->t == Type::INTEGER) {
                result = (Value*) heap.allocate<Integer>( ((Integer*) op2)->value + ((Integer*) op1)->value );
            
            } else if ( op1->t == Type::STRING || op2->t == Type::STRING ) {
                result = (Value*) heap.allocate<String>( op2->toString() + op1->toString() );

            } else {
                throw IllegalCastException("add");
            }

            opStack.push(result);

            }
            break;

        case Operation::Eq:
            {
            //cout << "Eq" << endl;
            Value* op1 = opStack.pop();
            Value* op2 = opStack.pop();
            if (op1->equals(op2) != op2->equals(op1)) {
                throw RuntimeException("equality must be transitive");
            }
            opStack.push((Value*) heap.allocate<Boolean>(op1->equals(op2)));

            }
            break;

        case Operation::Goto:
            {
            //cout << "Goto" << endl;

            int idx = i.operand0.value();
            nextPC += idx - 1;
            }
            break;

        case Operation::If:
            {
            //cout << "If" << endl;

            int idx = i.operand0.value();
            Value* op1 = opStack.pop();
            if (op1->t != Type::BOOLEAN) {throw IllegalCastException("Uninitialized to Bool"); }
            if ( ((Boolean*) op1)->value ) {
                nextPC += idx - 1; 
            }
            }
            break;

        case Operation::Dup:
            {
            //cout << "Dup" << endl;
            Value* op = opStack.pop();
            opStack.push(op); opStack.push(op);
            }
            break;

        case Operation::Swap:
            {
            //cout << "Swap" << endl;
            Value* first = opStack.pop();
            Value* second = opStack.pop();
            opStack.push(first); opStack.push(second);
            }
            break;

        case Operation::Pop:
            {
            //cout << "Pop" << endl;
            opStack.pop();
            }
            break;

        default:
            throw RuntimeException();
            break;
    }
}
