#pragma once

#include "instructions.h"
#include "../gc/gc.h"

#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <sstream>
#include <cstdint>
#include <map>

using namespace std;

template<class Type1, class Type2>
bool isInstanceOf(Type1* Instance) {
    return (dynamic_cast<Type2*>(Instance) != NULL);
}

enum Type {
  FUNCTION,
  NONE,
  INTEGER,
  STRING,
  BOOLEAN,
  REFERENCE,
  CLOSURE,
  RECORD,
};

class Value : public Collectable
{
public:
  const Type t;
  Value(Type tt) : t(tt) {}
  virtual string toString() = 0;
  virtual bool equals(Value* c) = 0;

  virtual ~Value() { };

  virtual void follow(CollectedHeap& heap) = 0;
  virtual int getSize() = 0;
};

class Constant : public Value
{
public:
  Constant(Type t) : Value(t) { }  
  virtual ~Constant() {};

  virtual void follow(CollectedHeap& heap) = 0;
  virtual int getSize() = 0;

  virtual Constant* copyTo(CollectedHeap& heap) = 0;
};

class None : public Constant
{
public:
  None() : Constant(NONE) { }
  string toString() { return "None"; }
  bool equals(Value* c) { return (c->t == Type::NONE); }
  virtual ~None() { }

  virtual void follow(CollectedHeap& heap) { return; } //no pointers
  
  virtual int getSize() {
    return sizeof(None);
  };

  virtual Constant* copyTo(CollectedHeap& heap) {
    return heap.allocate<None>();
  }

};

class Integer : public Constant
{
public:
  Integer(int32_t value) : value(value), Constant(INTEGER) { }
  int32_t value;
  string toString() { return to_string(value); }
  bool equals(Value* c) { return ( !(c->t == Type::INTEGER) ? false : (((Integer*) c)->value == this->value) ); }
  virtual ~Integer() { }

  virtual void follow(CollectedHeap& heap) { return; } // no pointers
  
  virtual int getSize() { 
    return sizeof(Integer);
  }

  virtual Constant* copyTo(CollectedHeap& heap) {
    return heap.allocate<Integer>(value);
  }

};

class String : public Constant
{
public:
    string value;
    String(std::string value) : value(value), Constant(STRING) { }
    string toString() { return value; }
    bool equals(Value* c) { return ( (!isInstanceOf<Value,String>(c)) ? false : (((String*) c)->value == this->value) ); }
    virtual ~String() { }

    virtual void follow(CollectedHeap& heap) { return; } // no pointers

    virtual int getSize() {
      return sizeof(String) + value.length()*sizeof(char);
    }

    virtual Constant* copyTo(CollectedHeap& heap) {
    return heap.allocate<String>(value);
  }
};

class Boolean : public Constant
{
public:
    bool value;
    Boolean(bool value) : value(value), Constant(BOOLEAN) { }
    string toString() { return (value ? "true" : "false"); }
    bool equals(Value* c) { ( (!isInstanceOf<Value,Boolean>(c)) ? false : (((Boolean*) c)->value == this->value) ); }
    
    virtual ~Boolean() { }

    virtual void follow(CollectedHeap& heap) { return; } //no pointers

    virtual int getSize() {
      return sizeof(Boolean);
    }

    virtual Constant* copyTo(CollectedHeap& heap) {
    return heap.allocate<Boolean>(value);
  }
}; 

class Function : public Value
{
public:

  string toString() { return "FUNCTION (raw)"; }
  bool equals(Value* c) {
    return c == this;
  };
  bool isNative;
  string nativeName;

  // List of functions defined within this function (but not functions defined inside of nested functions)
  std::vector<Function*> functions_;
 
 // List of constants used by the instructions within this function (but not nested functions)
  std::vector<Constant*> constants_;

 // The number of parameters to the function
  uint32_t parameter_count_;
  
  // List of local variables
  // The first parameter_count_ variables are the function's parameters
  // in their order as given in the paraemter list
  std::vector<std::string> local_vars_;
  
    // List of local variables accessed by reference (LocalReference)
  std::vector<std::string> local_reference_vars_;

  // List of the names of non-global and non-local variables accessed by the function
  std::vector<std::string> free_vars_;

  // List of global variable and field names used inside the function
  std::vector<std::string> names_;
    
  InstructionList instructions;

  virtual ~Function() {
    for (Function* f : functions_) {
      delete f;
    }
    for (Constant* c : constants_) {
      delete c;
    }
  }

  //Function() : Value(FUNCTION), functions(), constants(), parameter_count(0), local_vars(), local_reference_vars(), free_vars(), names(), instructions() {}
  
  Function() : Value(FUNCTION) {}

  Function( std::vector<Function*> functions,
            std::vector<Constant*> constants,
            uint32_t parameter_count,
            std::vector<std::string> local_vars,
            std::vector<std::string> local_reference_vars,
            std::vector<std::string> free_vars,
            std::vector<std::string> names,
            InstructionList instructions) :
            Value(FUNCTION),
            functions_(functions),
            constants_(constants),
            parameter_count_(parameter_count),
            local_vars_(local_vars),
            local_reference_vars_(local_reference_vars),
            free_vars_(free_vars),
            names_(names),
            instructions(instructions) {
              this->isNative = false;
              this->nativeName = "error";
            }


  virtual void follow(CollectedHeap& heap) { 
    for (Constant* c : constants_) {
      heap.markSuccessors(c);
    }
    for (Function* f : functions_) {
      heap.markSuccessors(f);
    }
  } // should never delete a function
  
  virtual int getSize() {return sizeof(Function); } // doesnt matter - functions allocated on C++ heap
};

class VarRef : public Value {
public:
  Value** refValue;
  VarRef() : Value(REFERENCE) {}
  VarRef(Value** v) : Value(REFERENCE) { this->refValue = v; }
  
  string toString() { return "REFERENCE"; }
  bool equals(Value* c) { 
  if (c->t != Type::REFERENCE) {return false;}
  return (  *(((VarRef*) c)->refValue) == *(this->refValue) );
  };

  virtual ~VarRef() {}

  virtual void follow(CollectedHeap& heap) { 
    heap.markSuccessors(*refValue); // need to keep my target marked 
  }

  virtual int getSize() { return sizeof(VarRef); } 

};

class Closure : public Value {
public:
  Function* func;
  std::vector<VarRef*> free_vars;

  Closure() : Value(CLOSURE) {}
  string toString() { return "FUNCTION"; }
  bool equals(Value* c) {
  if (c->t != Type::CLOSURE) {
    return false;
  } else {
    return ( c == this );
  }
}

  virtual ~Closure() {
    // no need to delete ref vars
    // dont delete function ever
  }

  virtual void follow(CollectedHeap& heap) {
    for (VarRef* vr : free_vars) {
      heap.markSuccessors(vr);
    }
    //heap.markSuccessors(func);
  }

  virtual int getSize() { return sizeof(Closure) + free_vars.size()*sizeof(VarRef); }

};

class RecordVal : public Value {
public:
  map<string,Value*> m;
  RecordVal() : Value(RECORD), m() {}

  bool equals(Value* c) { return (c == this); }

  string toString() {
    stringstream ss;
    ss << "{";
    map<string,Value*>::iterator it;

    for (it = m.begin(); it != m.end(); it++) {
      ss << it->first;
      ss << ":";
      ss << it->second->toString();
      ss << " ";
    }
    ss << "}";
    return ss.str();
  }

  Value* getValue(string x) {
    if (m.find(x) == m.end()) {
      return new None();
    } else {
      return m[x];
    }
  }

  void addValue(string x, Value* v) {
    if (m.find(x) == m.end()) {
      m[x] = v;
    } else {
      m[x] = v;
    }
  }

  virtual ~RecordVal() {}

  virtual void follow(CollectedHeap& heap) {
    for (auto i : m) {
      heap.markSuccessors(i.second);
    }
  }

  virtual int getSize() {

    int s = sizeof(RecordVal);

    for (auto i : m) {
      s += i.first.length()*sizeof(char);
      s += sizeof(void*);
    }

    return s;
  }

};

class RecordValOrder : public Value {
public:
  vector<string> m1;
  vector<Value*> m2;
  RecordValOrder() : Value(RECORD), m1(), m2() {}

  bool equals(Value* c) { return (c == this); }

  string toString() {
    stringstream ss;
    ss << "{";

    for (int i = 0; i < m1.size(); i++) {
      ss << m1[i];
      ss << ":";
      ss << m2[i]->toString();
      ss << " ";
    }
    ss << "}";
    return ss.str();
  }

  Value* getValue(string x) {
    ptrdiff_t s = find(m1.begin(), m1.end(), x) - m1.begin();
    if (s >= m1.size()) {
      return new None();
    }
    return m2[(int) s];
  }

  void addValue(string x, Value* v) {
    ptrdiff_t s = find(m1.begin(), m1.end(), x) - m1.begin();
    if (s >= m1.size()) {
      m1.push_back(x);
      m2.push_back(v);
    } else {
      m2[(int) s] = v;
    }
  }

  virtual ~RecordValOrder() {}

  virtual void follow(CollectedHeap& heap) {
    for (Value* v : m2) {
      heap.markSuccessors(v);
    }

  }

  virtual int getSize() { 

    int s = sizeof(RecordValOrder);
    for (string ss : m1) {
      s += ss.length()*sizeof(char);
      s += sizeof(void*);
    }

    return s;
  }


};









