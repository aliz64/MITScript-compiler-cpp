#include <iostream>
#include <map>
#include <stack>
#include "types.h"
#include "instructions.h"

using namespace std;

///////////////from stackExchange - just exposing stack iterators////
template<typename T, typename Container = std::deque<T>>
class iterable_stack : public std::stack<T, Container> {
    using std::stack<T, Container>::c;
public:
    // expose just the iterators of the underlying container
    auto begin() { return std::begin(c); }
    auto end() { return std::end(c); }
    auto begin() const { return std::begin(c); }
    auto end() const { return std::end(c); }
};
//////////////////////////////////////////////////////////////////

class InterpreterException {
public:
    string msg;
    InterpreterException(string x) { msg = x; }
    string message() { return msg; };
};

class UninitializedVariableException2 : public InterpreterException {
public:
    UninitializedVariableException2() : InterpreterException("UninitializedVariableException") { }
    UninitializedVariableException2(string x) : InterpreterException("UninitializedVariableException: "+x) {}
};

class IllegalCastException : public InterpreterException {
public:
    IllegalCastException() : InterpreterException("IllegalCastException") { }
    IllegalCastException(string x) : InterpreterException("IllegalCastException: "+x) { }
};

class IllegalArithmeticException : public InterpreterException {
public:
    IllegalArithmeticException() : InterpreterException("IllegalArithmeticException") { }
    IllegalArithmeticException(string x) : InterpreterException("IllegalArithmeticException: "+x) { }
};

class InsufficientStackException : public InterpreterException {
public:
    InsufficientStackException() : InterpreterException("InsufficientStackException") { }
    InsufficientStackException(string x) : InterpreterException("InsufficientStackException: "+x) { }
};

class RuntimeException : public InterpreterException {
public:
    RuntimeException() : InterpreterException("RuntimeException") { }
    RuntimeException(string x) : InterpreterException("RuntimeException: "+x) { }
};

class UnimplementedException : public InterpreterException {
public:
    UnimplementedException() : InterpreterException("UnimmplementedException") {}
    UnimplementedException(string x) : InterpreterException("UnimmplementedException: "+x) {}
};

class OpStack {
public:
    iterable_stack<Value*>* stk;
    OpStack() {
        this->stk = new iterable_stack<Value*>;
    }

    ~OpStack() {
        delete stk;
    }

    Value* pop() {
        if ( stk->size() == 0 ) {
            throw InsufficientStackException();
        }
        Value* r = stk->top();
        stk->pop();
        return r;
    }

    void push(Value* v) {
        stk->push(v);
    }

    Value* top() {
        return stk->top();
    }

    void printStack() {
        int i = 0;
        for (Value* v : *(this->stk) ) {
            cout << i << " " << v->toString() << endl; 
            i++;
        }
    }
};

class Frame : public Collectable {
public:

    vector<string> initialized;

    vector<Value*> constants;
    vector<Function*> functions;
    vector<Value*> local_vars;
    vector<VarRef*> local_ref_vars;
    vector<VarRef*> free_vars;
    map<string, Value*> names;
    vector<string> namesIdx;
    int parameter_count;
    Frame() : constants(),
              initialized(),
              functions(),
              local_vars(),
              local_ref_vars(),
              free_vars(),
              names(),
              namesIdx(),
              parameter_count(0) {}

    
    virtual string toString() { return "FRAME"; }

    virtual void follow(CollectedHeap& heap) {
        for (Value* c : constants) {
            heap.markSuccessors(c);
        }
        for (Value* lv : local_vars) {
            heap.markSuccessors(lv);
        }
        for (VarRef* lrv : local_ref_vars) {
            heap.markSuccessors(lrv);
        }
        for (VarRef* fv : free_vars) {
            heap.markSuccessors(fv);
        }
        for (auto np : names) {
            heap.markSuccessors(np.second);
        }
    }

    virtual int getSize() {

        return 0;

        int s = sizeof(Frame);
        
        for (Value* c : constants) {     
            s += s += sizeof(void*);
        }

        for (Value* lv : local_vars) {
            s += sizeof(void*);
        }
        for (VarRef* lrv : local_ref_vars) {
            s += s += sizeof(void*);
        }
        for (VarRef* fv : free_vars) {
            s += s += sizeof(void*);
        }
        for (auto np : names) {
            s += (np.first).capacity();
            s += s += sizeof(void*);
        }
        return s;
    }


};


class Interpreter {
public:
    bool first;
    stack<Function*> stackF;
    iterable_stack<Frame*> gamma;
    Frame* gFrame;
    Value* rval;
    int PC;
    int nextPC;
    OpStack opStack;

    bool gcFirst;

    Function* main;

    CollectedHeap& heap;

    void garbageCollect();
    Interpreter(CollectedHeap& heapp);
    void execute(Instruction i);
    Value* interpret(Function* f);


};




