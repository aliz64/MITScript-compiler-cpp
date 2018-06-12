/*!
This contains the declarations as well as the implementation
of the Abstract Syntax Tree for the program
*/

#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <queue>
#include <string.h>

#define Assert(cond, msg) if(!(cond)){ std::cerr<<msg<<endl; throw SystemException("Bad stuff"); }

using namespace std;

class SystemException {
    string msg_;
public:
    SystemException(const string& msg) :msg_(msg) {}
};

class Visitor;

/*!
Abstract class to define a generic AST node
All other specific nodes will inherit from this
*/
class AST_node {
public:
    /*!
    Pure Virtual Accept Method much be overridden by specific
    subclasses of the AST_node

    @param v - a visitior object which this node accepts
    */
    virtual void accept(Visitor& v) = 0;
};

/*!
Abstract class to define a statement in our MITScript program
All other specific types of statements will inherit from this
*/
class Statement: public AST_node {
public:
};

/*!
Defines a piece of code which is just a collection of statements
*/
class StatementList : public Statement {
public:
    // holds the lists of statements as a vector of pointers to statements
    deque<Statement*>* statementList;
    
    // Conctructor: just initialize an empty list
    StatementList() {
        this->statementList = new deque<Statement*>();   
    }
    
    // Add a statement to the end of the list
    // @param last - the Statement to add to the end of this list
    void add(Statement* last) {
        this->statementList->emplace_back(last);
            
    }
    
    // Override the virtual method from the abstract Statement parent class
    void accept(Visitor& v);
};

/*!
Defines a Block of code which is just a StatementList with 
curly braces around it
*/
class Block : public Statement {
public:
    // The statement list
    StatementList* stlst;

    // Constructor
    // @param lst - the statement list in this block
    Block(StatementList* lst) {
        this->stlst = lst;
    }

    // Constructor for empty block
    Block() {
        this->stlst = new StatementList();
    }

    // Override the method from the parent class
    void accept(Visitor& v);
};

/*!
Global class representing the declaration of a variable as Global
*/
class Global : public Statement {
public:
    // Holds the name of the variable that has been declared global
    string varname;

    // Constructor
    // @param varname - the name of the variable to be declared global
    Global(char* varname) {
        this->varname = varname;
    }

    // Overriding the virtual method from the abstract Statement parent class
    void accept(Visitor& v);
};

/*!
An Abstract expression class representing all expressions
All other specific expressions will inherit from this
*/
class Expression : public AST_node {
public:
};


/*!
An abstract class representing an expression of type LHS
which is the left hand side of an assignment
*/
class LHS : public Expression {
public:
};

/*!
A Field Deferred LHS (representing field referencing)
*/
class FieldDefer : public LHS {
public:
    // The LHS base
    LHS* base;

    // The name of the field
    string field;

    /*!
    Constructor
    @param b - the LHS base
    @param f - the field referenced
    */
    FieldDefer(LHS* b, char* f) {
        this->field = f;
        this->base = b;
    }

    // Overriding the accept method of the abstract parent LHS class
    void accept(Visitor& v);
};

/*!
An Indexed Expression LHS (representing indexing)
*/
class IndexedExpr : public LHS {
public:
    // The LHS base
    LHS* base;

    // The expression that acts as the index
    Expression* index;

    /*!
    Constructor
    @param b - the base expression
    @param i - the expression used to index into the base
    */
    IndexedExpr(LHS* b, Expression* i) {
        this->index = i;
        this->base = b;
    }

    //Overriding the method inherited from the abstract LHS parent class
    void accept(Visitor& v);
};

/*!
A list of expressions; used to represent arguments in a method call
*/
class ExpressionList : public AST_node {
public:
    // The list of expressions stored as a vector
    vector<Expression*>* exList;
    
    // Constructor initiazing an empty list 
    ExpressionList() {
        this->exList = new vector<Expression*>;
    }
    
    // Constructor initializing a list with given expression
    // @param expr - the initial expression in the expression list
    ExpressionList(Expression* expr) {
        this->exList = new vector<Expression*>({expr});  
    }
    
    // Adds an expression to the end of this expression list
    // @param last - the expression to be added 
    void add(Expression* last) {
        this->exList->push_back(last);
            
    }
    
    // Overriding the accept method from the abstract parent AST_node class
    void accept(Visitor& v);
};

/*!
A binary expression between two operands and an operation
*/
class BinaryExpression : public Expression {
public:
    // The string repsresentation of the operation
    string op;

    // The left and right operands
    Expression* left;
    Expression* right;
    
    /*!
    Constructor
    @param l - the left operand Expression
    @param r - the right operand Expression
    @param o - a string representation of the operand
    */
    BinaryExpression(Expression* l, Expression* r, string o) {
        this->right = r;
        this->left = l;
        this->op = o;
    }   
    
    // Overriding the accept method from the abstract parent Expression class
    void accept(Visitor& v);
};

/*!
A unary expression and its operand
*/
class UnaryExpression : public Expression {
public:
    // The string representation of the operand
    string op;

    // The expression the unary operator is applied to
    Expression* ex;
    
    /*!
    Constructor
    @param e   - the Expression the operand is applied to
    @param ope - the string representation of the unary operator
    */
    UnaryExpression(Expression* e, string ope) {
        this->op = ope;
        this->ex = e;
    }

    // Overriding the accept method of the abstract parent Expression class
    void accept(Visitor& v);
};

/*!
A list fo names; used in function declerations
*/
class NameList : public AST_node {
public:
    // The list of char*/string representation of the names
    vector<string> list;
    
    // Constructor initializing an empty list of names
    NameList() {
        this->list = {};
    }
    
    // Constructor initializing a list with the specified name
    // @param str - the name to initalize the list with
    NameList(const char* str) {
        string s = str;
        this->list = {s}; 
    }
    
    // Adds a name to the end of the list
    // @param last - the name to add to the end of the list
    void add(char* last) {
        string s = last;
        this->list.push_back(s);
            
    }
    
    // Overriding the accept method of the abstact parent AST_node class
    void accept(Visitor& v);
};

/*!
A fucntion decleration expression
*/
class FunctionDeclaration : public Expression {
public:
    // The list of arguments to the function
    NameList* args;

    // The body of the function
    Block* body;

    /*!
    Constructor initializing from a name list and body
    @param nl - the name list for the arguments to the function
    @param b - the block body of the funciton
    */
    FunctionDeclaration(NameList* nl, Block* b) {
        this->body = b;
        this->args = nl;
    }
    
    /*!
    Constructor initializing from a body only
    */
    FunctionDeclaration(Block* b) {
        this->body = b;
        this->args = new NameList();
    }

    // Overriding the accept method of the abstract parent Expression class
    void accept(Visitor& v);
};

/*!
A record expression
*/
class Record : public Expression {
public: 
    // The records in the Record mapped from names to expresions
    vector<string> m1;
    vector<Expression*> m2;

    // Conctructor for creating a record with one entry
    // @param s - the name of the key in the record
    // @param e - the name of the expression that the key maps to
    Record(char* s, Expression* e) {
        string ss = s;
        m1 = {ss}; m2 = {e};
    }
    
    // Constructor for creating an empty record
    Record() {
        m1 = {}; m2 = {};
    }

    /*!
    Adds a key/value pair to the record
    @param s - the name of the variable to be added
    @param e - the expression that variable maps to
    */
    void add(char* s, Expression* e) {
        string ss = s;
        m1.push_back(ss); m2.push_back(e);
    }

    // Overriding the virtual accept method of the parent abstract Expression class
    void accept(Visitor& v);
};

/*!
Represents an assignment operation
*/
class Assignment : public Statement {
public:
    // The LHS of the assignment
    LHS* lhs;

    // The expression to be assigned
    Expression* expression;

    /*!
    Constructor
    @param expr - the expression to be assigned to the lhs
    @param lhs - the left hand side of the assignment statement
    */
    Assignment(Expression* expr, LHS* lhs) {
        this->expression = expr;
        this->lhs = lhs;
    }
    
    // overriding the virtual method in abstract parent statement class
    void accept(Visitor& v);
};

/*!
A function call statement
*/
class CallStatement : public Statement {
public:
    // The call expression
    Expression* call;

    // Constructor
    // @param c - the call expression
    CallStatement(Expression* c) {this->call = c;}
    
    // Overriding the virtual accept method of the abstract parent statement class
    void accept(Visitor& v);
};

/*!
A function call expression
*/
class Call : public Expression {
public:
    // The left hand side
    LHS* lhs;

    // An expression list for the arguments to be passed in
    ExpressionList* exl;
    
    /*!
    Constructor for an empty function call
    @param l - the LHS representing the method to be called
    */
    Call(LHS* l) {
        this->lhs = l;
        this->exl = new ExpressionList();
    }   
    
    /*!
    Constructor
    @param el - the expression list of the called arguments
    @param l - the LHS which is the function to be called
    */
    Call(ExpressionList* el, LHS* l) {
        this->exl = el;
        this->lhs = l;
    }

    // Overriding the virtual accept method in abstract parent expression class
    void accept(Visitor& v);
};

/*!
A Variable Name
*/
class Name : public LHS {
public:
    // The string representation of the name
    string name;
    
    // Constructor
    // @param s - the name of the variable
    Name(const char* s) {this->name = s;}
    
    // overriding the accept method from the parent LHS abstract class
    void accept(Visitor& v);
};

/*!
An IF statement
*/
class IfStatement : public Statement {
public:
    // The expression on which the IF is conditioned
    Expression* condition;

    // The if and else blocks
    Block* ifPart;
    Block* elsePart;

    /*!
    Constructor for IF only statement
    @param e - the expression to be conditioned on
    @param b - the block to be executed if the condition is true
    */
    IfStatement(Expression* e, Block* b) {
        this->condition = e;
        this->ifPart = b;
        this->elsePart = new Block();
    }

    /*!
    Constructor for IF ELSE statement
    @param e - the expression to be conditioned on
    @param b - the block to be executed if the condition is true
    @param b2 - the block to be executed if the conditon is false
    */
    IfStatement(Expression* e, Block* b, Block* b2) {
        this->condition = e;
        this->ifPart = b;
        this->elsePart = b2;
    }

    // Overriding the virtual method from abstract parent Statement class
    void accept(Visitor& v);
};

/*!
A While Loop
*/
class WhileLoop : public Statement {
public:
    // The condition on which the wile loop is based on
    Expression* condition;

    // The body of the while loop
    Block* body;

    // Constructor
    // @param e - the expression if true the loop executes
    // @param b - the block of code to be executed if condition true
    WhileLoop(Expression* e, Block* b) {
        this->condition = e;
        this->body = b;    
    }

    // Overriding the virtual accept method in abstract parent Statement class
    void accept(Visitor& v);
};

/*!
A Return statement
*/
class Return : public Statement {
public:
    // The expression to be returned
    Expression* e;

    // Constructor
    // @param e - the expression to be returned
    Return(Expression* e) {
        this->e = e;
    }

    // Overriding the virtual accept method in abstract parent Statement
    void accept(Visitor& v);
};

/*!
An Integer Constant Exppression
*/
class IntConst : public Expression {
public:
    // The value of the integer
    int i;

    // Constructor
    // @param i - the integer represented by this integer constant
    IntConst(int i) { this->i = i;}

    // Overriding the virtual accept method of the parent abstract Expression class
    void accept(Visitor& v);
};

/*!
A String Constant Exppression
*/
class StrConst : public Expression {
public:
    // The string
    string str;

    // Constructor
    // @apram s - the string to be represented
    StrConst(const char* s) {
        this->str = s;
        // remove leading and ending quotes
        str = str.substr(1,str.length()-2);
    }

    // Overriding the virtual accept method in the abstract parent expression class
    void accept(Visitor& v);
};

/*!
A Boolean Constant Exppression
*/
class BoolConst : public Expression {
public:
    // The boolean
    bool b;

    // Constructor
    // @param bb - the boolean to be represented
    BoolConst(bool bb) { this->b = bb; }

    // Overrding the accept methof of the abstract parent Expression class
    void accept(Visitor& v);
};

/*!
A None Constant Exppression
*/
class NoneConst : public Expression {
public:
    // Overriding the accept method of the abstract parent Expression class
    void accept(Visitor& v);
};


class SPrint : public Block {
public:

    void accept(Visitor& v);
};

class SICast : public Block {
    void accept(Visitor& v);
};

