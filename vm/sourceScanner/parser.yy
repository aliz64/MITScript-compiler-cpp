/*This is the Parser File (builds on top of the Scanner in lexer.lex)
  for MIT 6.S081 (by Ali Zartash, Spring 2018)*/

%code requires {
#include <iostream>
#include <string>
#define YY_DECL int yylex (YYSTYPE* yylval, YYLTYPE * yylloc, yyscan_t yyscanner)
#ifndef FLEX_SCANNER
#include "lexer.h"
#endif

using namespace std;

// The macro below is used by bison for error reporting
// it comes from stackoverflow
// http://stackoverflow.com/questions/656703/how-does-flex-support-bison-location-exactly
#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->first_column = yylloc->last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc->last_line++; \
            yylloc->last_column = 0; \
        } \
        else { \
            yylloc->last_column++; \
        } \
    }

// Additional header files can be put here
#include "AST.h"
#include <cassert>
}

%define api.pure full
%parse-param {yyscan_t yyscanner} {Statement*& out}
%lex-param {yyscan_t yyscanner}
%locations
%define parse.error verbose
%code provides {
YY_DECL;
int yyerror(YYLTYPE * yylloc, yyscan_t yyscanner, Statement*& out, const char* message);
}


// The union directive defines a union type that will be used to store
// the return values of all the parse rules. We have initialized for you
// with an intconst field that you can use to store an integer, and a
// stmt field with a pointer to a statement. Note that one limitation
// is that you can only use primitive types and pointers in the union.
%union {
        // Constants
        int                intconst;
        char*              strconst;
        bool               boolconst;
        
        // Statements
        Statement*         stmt;
        StatementList*     stmtLst;
        Block*             blk;

        // Lists
        ExpressionList*    exprList;
        NameList*          namelist;

        // Expressions
        Expression*        expr;
        LHS*               lhs;
        Record*            record;
}

// Below is where you define your tokens and their types.
// for example, we have defined for you a T_int token, with type intconst
// the type is the name of a field from the union above
%token<intconst>    T_int
%token<strconst>    T_str
%token<boolconst>   T_bool
%token<strconst>    NAME

%token GLOBAL ASSN LEFT_BRA RIGHT_BRA IF WHILE
%token NOT SEMICOLON LEFT_PARAN RIGHT_PARAN
%token RETURN FUN PLUS MINUS OR RIGHT_SQR
%token LEFT_SQR MULT DIVIDE COLON AND NONE_CONST
%token GRTNEQ LSTNEQ GRTN LSTN EQ COMMA ELSE DOT

// Use the %type directive to specify the types of AST nodes produced by each production.
// For example, you will have a program non-terimnal in your grammar, and it will
// return a Statement*. As with tokens, the name of the type comes
// from the union defined earlier.

// Statement Types
%type<stmt>             Program
%type<stmt>             Statement
%type<stmt>             Global
%type<stmt>             Assignment
%type<stmt>             IfStatement
%type<stmt>             WhileLoop
%type<stmt>             Return
%type<stmt>             CallStatement
%type<stmtLst>          StatementList
%type<blk>              Block

//
%type<exprList>         CalledArgList
%type<namelist>         ArgList
%type<record>           RecordList
%type<lhs>              LHS

// Expression Types
%type<expr>             Expression
%type<expr>             Call
%type<expr>             Record
%type<expr>             Function
%type<expr>             Boolean
%type<expr>             Conjunction
%type<expr>             Arithmetic
%type<expr>             Product
%type<expr>             BoolUnit
%type<expr>             Predicate
%type<expr>             Unit
%type<expr>             Unit_UNSIGNED
%type<expr>             Constant

// The start symbol is Program
%start Program

// You must also define any associativity directives that are necessary
// to resolve ambiguities and properly parse the code.
%%

// Grammar Rules written here:

Program:
    StatementList { $$ = (Statement*) $1; out = $$; /*out is the output variable*/ }
    ;

StatementList:
    StatementList Statement { $$->add($2); }
    | { $$ = new StatementList(); }
    ;

Block:
    LEFT_BRA StatementList RIGHT_BRA { $$ = new Block($2); }
    ;

Statement:
    Global { $$ = $1; }
    | Assignment { $$ = $1; }
    | CallStatement { $$ = $1; }
    | IfStatement { $$ = $1; }
    | WhileLoop { $$ = $1; }
    | Return { $$ = $1; }
    ;

Global:
    GLOBAL NAME SEMICOLON { $$ = new Global($2); }
    ;

Assignment:
    LHS ASSN Expression SEMICOLON 
    { $$ = new Assignment($3, $1); }
    ;

CallStatement:
    Call SEMICOLON {$$ = new CallStatement($1); }
    ;

IfStatement:
    IF LEFT_PARAN Expression RIGHT_PARAN Block ELSE Block
    { $$ = new IfStatement($3, $5, $7); }
    | IF LEFT_PARAN Expression RIGHT_PARAN Block
    { $$ = new IfStatement($3, $5); }
    ;

WhileLoop:
    WHILE LEFT_PARAN Expression RIGHT_PARAN Block
    { $$ = new WhileLoop($3, $5); }
    ;

Return:
    RETURN Expression SEMICOLON { $$ = new Return($2); }
    ;

Expression:
    Function { $$ = $1; }
    | Boolean { $$ = $1; }
    | Record { $$ = $1; }
    ;

Function:
    FUN LEFT_PARAN ArgList RIGHT_PARAN Block
    { $$ = new FunctionDeclaration($3, $5); }
    | FUN LEFT_PARAN RIGHT_PARAN Block
    { $$ = new FunctionDeclaration($4); }
    ;

ArgList:
    NAME { $$ = new NameList($1); }
    | ArgList COMMA NAME { $$->add($3); }
    ;

Boolean:
    Conjunction { $$ = $1; }
    | Conjunction OR Boolean
    { $$ = new BinaryExpression( $1, $3, "|"); }
    ;


Conjunction:
    BoolUnit { $$ = $1; }
    | BoolUnit AND Conjunction
    {$$ = new BinaryExpression($1, $3, "&"); }
    ;

BoolUnit:
    NOT Predicate { $$ = new UnaryExpression($2, "!"); }
    | Predicate { $$ = $1; }
    ;

Predicate:
    Arithmetic { $$ = $1; }
    | Arithmetic GRTN Arithmetic
    { $$ = new BinaryExpression($1, $3, ">"); }
    | Arithmetic LSTN Arithmetic
    { $$ = new BinaryExpression($1, $3, "<"); }
    | Arithmetic GRTNEQ Arithmetic
    { $$ = new BinaryExpression($1, $3, ">="); }
    | Arithmetic LSTNEQ Arithmetic
    { $$ = new BinaryExpression($1, $3, "<="); }
    | Arithmetic EQ Arithmetic
    { $$ = new BinaryExpression($1, $3, "=="); }
    ;

Arithmetic:
    Product { $$ = $1; }
    | Arithmetic PLUS Product
    { $$ = new BinaryExpression($1, $3, "+"); }
    | Arithmetic MINUS Product
    { $$ = new BinaryExpression($1, $3, "-"); }
    ;

Product:
    Unit { $$ = $1; }
    | Product MULT Unit
    { $$ = new BinaryExpression($1, $3, "*"); }
    | Product DIVIDE Unit
    { $$ = new BinaryExpression($1, $3, "/"); }
    ;

Unit:
    Unit_UNSIGNED { $$ = $1; }
    | MINUS Unit_UNSIGNED { $$ = new UnaryExpression($2, "-"); }
    ;

Unit_UNSIGNED:
    LHS { $$ = $1; }
    | Constant { $$ = $1; }
    | Call { $$ = $1; }
    | LEFT_PARAN Boolean RIGHT_PARAN { $$ = $2; }
    ;

LHS:
    NAME { $$ = new Name($1); }
    | LHS DOT NAME { $$ = new FieldDefer($1, $3); }
    | LHS LEFT_SQR Expression RIGHT_SQR { $$ = new IndexedExpr($1, $3); }
    ;

Call:
    LHS LEFT_PARAN CalledArgList RIGHT_PARAN { $$ = new Call($3, $1); }
    | LHS LEFT_PARAN RIGHT_PARAN { $$ = new Call($1); }
    ;

CalledArgList:
    Expression { $$ = new ExpressionList($1); }
    | CalledArgList COMMA Expression { $$->add($3); }
    ;


Record:
    LEFT_BRA RIGHT_BRA {$$ = new Record(); }
    | LEFT_BRA RecordList RIGHT_BRA {$$ = $2;}
    ;

RecordList:
    NAME COLON Expression SEMICOLON
    { $$ = new Record($1, $3); }
    | RecordList NAME COLON Expression SEMICOLON
    { $$->add($2, $4); }
    ;

Constant:
    T_int { $$ = new IntConst($1); }
    | T_str { $$ = new StrConst($1); }
    | T_bool { $$ = new BoolConst($1); }
    | NONE_CONST { $$ = new NoneConst(); }
    ;

%%

// Error reporting function. You should not have to modify this.
int yyerror(YYLTYPE * yylloc, void* p, Statement*& out, const char*  msg) {
  cout<<"Error in line "<<yylloc->last_line<<", col "<<yylloc->last_column<<": "<<msg;
  return 0;
}
