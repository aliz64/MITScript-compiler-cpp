#pragma once
#include "./sourceScanner/AST.h"
#include "instructions.h"
#include "types.h"

using namespace std;



class BasicBlock {
public:
    virtual ~BasicBlock() {
        delete instructions;
    };

    bool isBoolB;
    bool isWhile;

    BasicBlock* trueNext;
    BasicBlock* falseNext;
    BasicBlock* whileNext;

    vector<Instruction*>* instructions;
    BasicBlock* next;

    BasicBlock() {
        this->isBoolB = false;
        this->isWhile = false;
        this->trueNext = NULL;
        this->falseNext = NULL;
        this->whileNext = NULL;

        this->instructions = new std::vector<Instruction*>;
        this->next = NULL;
    }

    BasicBlock(Instruction* i) {
        this->isBoolB = false;
        this->isWhile = false;
        this->trueNext = NULL;
        this->falseNext = NULL;

        this->instructions = new std::vector<Instruction*>;
        this->instructions->push_back(i);
        this->next = NULL;
    }

    void join(BasicBlock* b) {
        for (Instruction* i : *(b->instructions) ) {
            this->instructions->push_back(i);
        }
        this->next = b->next;
    }
};

class FuncBB {
public:

    virtual ~FuncBB() {
        delete names; delete local_vars; delete childsF; delete constants;
        delete local_ref_vars; delete free_vars; delete record_names;

        delete cfg;
    }

    bool isNative;
    string nativeName;

    BasicBlock* cfg;
    BasicBlock* latestB;

    BasicBlock* refPoint; // add push references to this point

    FuncBB* parentF;
    vector<FuncBB*>* childsF;

    vector<string>* names;
    vector<string>* local_vars;
    int paramcount;

    vector<Constant*>* constants;
    vector<string>* local_ref_vars;
    vector<string>* free_vars;

    vector<string>* record_names;

    BasicBlock* scanCFG;


    FuncBB () {

        this->scanCFG = NULL;

        this->refPoint == NULL;

        this->isNative = false;
        this->nativeName = "error";

        this->cfg = new BasicBlock();
        this->latestB = cfg;

        this->parentF = NULL;
        this->childsF = new vector<FuncBB*>;

        this->names = new vector<string>;
        this->local_vars = new vector<string>;
        this->paramcount = 0;

        this->constants = new vector<Constant*>; this->constants->push_back((Constant*) new None());
        this->local_ref_vars = new vector<string>;
        this->free_vars = new vector<string>;

        this->record_names = new vector<string>;
    }
};


class cfgHeap {
public:
    vector<BasicBlock*> b;

    template<typename T, typename... ARGS>
    T* allocate(ARGS... args)
    {
        T* object = new T(args...);

        b.push_back(object);

        return object;

    }

    void clean() {
        for (BasicBlock* bb : b) {
            delete bb;
        }
    }

};

class FuncBBheap {
public:
    vector<FuncBB*> b;

    template<typename T, typename... ARGS>
    T* allocate(ARGS... args)
    {
        T* object = new T(args...);

        b.push_back(object);

        return object;

    }

    void clean() {
        for (FuncBB* bb : b) {
            delete bb;
        }
    }


};

class InstructionHeap {
public:
    vector<Instruction*> b;

    template<typename T, typename... ARGS>
    T* allocate(ARGS... args)
    {
        T* object = new T(args...);

        b.push_back(object);

        return object;

    }

    void clean() {
        for (Instruction* bb : b) {
            delete bb;
        }
    }


};