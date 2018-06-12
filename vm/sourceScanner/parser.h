/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_SOURCESCANNER_PARSER_H_INCLUDED
# define YY_YY_SOURCESCANNER_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 4 "./sourceScanner/parser.yy" /* yacc.c:1909  */

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

#line 75 "./sourceScanner/parser.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_int = 258,
    T_str = 259,
    T_bool = 260,
    NAME = 261,
    GLOBAL = 262,
    ASSN = 263,
    LEFT_BRA = 264,
    RIGHT_BRA = 265,
    IF = 266,
    WHILE = 267,
    NOT = 268,
    SEMICOLON = 269,
    LEFT_PARAN = 270,
    RIGHT_PARAN = 271,
    RETURN = 272,
    FUN = 273,
    PLUS = 274,
    MINUS = 275,
    OR = 276,
    RIGHT_SQR = 277,
    LEFT_SQR = 278,
    MULT = 279,
    DIVIDE = 280,
    COLON = 281,
    AND = 282,
    NONE_CONST = 283,
    GRTNEQ = 284,
    LSTNEQ = 285,
    GRTN = 286,
    LSTN = 287,
    EQ = 288,
    COMMA = 289,
    ELSE = 290,
    DOT = 291
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 51 "./sourceScanner/parser.yy" /* yacc.c:1909  */

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

#line 145 "./sourceScanner/parser.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (yyscan_t yyscanner, Statement*& out);
/* "%code provides" blocks.  */
#line 40 "./sourceScanner/parser.yy" /* yacc.c:1909  */

YY_DECL;
int yyerror(YYLTYPE * yylloc, yyscan_t yyscanner, Statement*& out, const char* message);

#line 176 "./sourceScanner/parser.h" /* yacc.c:1909  */

#endif /* !YY_YY_SOURCESCANNER_PARSER_H_INCLUDED  */
