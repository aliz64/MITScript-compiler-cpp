/*!
This file contains the header information for
Value which represents a return value in the MITScript Language
By Ali Zartash (for 6.S081 Spring 2017)
*/

#pragma once
#include <unordered_map>
#include <map>
#include <string>
#include <unordered_set>
#include <iostream>
#include <sstream>
#include "AST.h"
using namespace std;

class Value {
public:
    virtual string toString() = 0;
};

class StackFrame : public Value {
public:
    map<string, Value*>* frame;
    unordered_set<string>*  globalVars;
    StackFrame* gFrame;
    StackFrame* parent;

    StackFrame() {
        frame = new map<string, Value*>;
        globalVars = new unordered_set<string>;
        gFrame = NULL; parent = NULL;
    }

    string toString() {
        stringstream ss;
        ss << "StrackFrame: " << endl;
        ss << "{ ";
        map<string,Value*>::iterator it;
        for (it = frame->begin(); it != frame->end(); it++) {
            ss << it->first;
            ss << ":";
            ss << it->second->toString();
            ss << " ; ";
        }
        ss << " }";
        return ss.str();
    }
};

class Bool : public Value {
public:
    bool _b;
    Bool(bool b) : _b(b) {}

    string toString() {
        stringstream ss;
        _b? ss << "true" : ss << "false";
        return ss.str();
    }
};

class Integer : public Value {
public:
    int _n;
    Integer(int n) : _n(n) {}

    string toString() {
        stringstream ss;
        ss << _n;
        return ss.str();
    }
};

class String : public Value {
public:
    string _s;
    String(string s) : _s(s) {}

    string toString() {
        return _s;
    }
};

class RecordVal : public Value {
public:
    map<string,Value*>* _record;
    RecordVal() {
        _record = new map<string,Value*>;
    }

    void add(string s, Value* v) {
        _record->insert({s, v});
    }

    string toString() {
        stringstream ss;
        ss << "{";
        map<string,Value*>::iterator it;
        for (it = _record->begin(); it != _record->end(); it++) {
            ss << it->first;
            ss << ":";
            ss << it->second->toString();
            ss << " ";
        }
        ss << "}";
        return ss.str();
    }
};

class Function : public Value {
public:
    StackFrame* frame;
    Statement* body;
    NameList* args;
    Function(StackFrame* a, NameList* x, Statement* s) {
        frame = a;
        body = s;
        args = x;
    }

    string toString() {
        stringstream ss;
        ss << "FUNCTION";
        return ss.str();
    }
};

class None : public Value {
public:
    None() {}

    string toString() {
        stringstream ss;
        ss << "None";
        return ss.str();
    }
};

class ReturnVal : public Value {
public:
    Value* val;
    ReturnVal(Value* v) {val = v;}

    string toString() {
        throw 1;
    }
};