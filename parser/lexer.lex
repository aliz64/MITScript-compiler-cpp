%{
// This is the Flex Scanner for MIT 6.S081 (By Ali Zartash, Spring 2018)
// Delaration and Option Settings Section
// Additional Header Files can be declared here
#include <string>
#include "parser.h"
using std::string;
%}
%option reentrant
%option noyywrap
%option never-interactive
%{
// Initial declarations
// Definitions for named regular expressions used below
// (for example int_const and whitespace)
%}
whitespace   ([ \t\n]*)
comment  "//".*"\n"
int_const [0-9][0-9]*
string_const \"([^\"\\]|"\\\\"|"\\\""|"\\n"|"\\t")*\"
VAR_NAME ("_"|[A-Za-z])("_"|[A-Za-z0-9])*

%%

%{
// Pattern-Action section of this Flex Scanner
%}
"\."                    { return DOT; }
"global"                { return GLOBAL; }
"{"                     { return LEFT_BRA; }
"}"                     { return RIGHT_BRA; }
"if"                    { return IF; }
"else"                  { return ELSE; }
"while"                 { return WHILE; }
"!"                     { return NOT; }
";"                     { return SEMICOLON; }
"("                     { return LEFT_PARAN; }
")"                     { return RIGHT_PARAN; }
","                     { return COMMA; }
"return"                { return RETURN; }
"fun"                   { return FUN;}
"+"                     { return PLUS; }
"-"                     { return MINUS; }
"|"                     { return OR; }
"]"                     { return RIGHT_SQR; }
"["                     { return LEFT_SQR; }
"*"                     { return MULT; }
"/"                     { return DIVIDE; }
":"                     { return COLON; }
"&"                     { return AND; }
"None"                  { return NONE_CONST; }
("true")                { yylval->boolconst = true; return T_bool; }
("false")               { yylval->boolconst = false; return T_bool; }
">="                    { return GRTNEQ; }
"<="                    { return LSTNEQ; }
"=="                    { return EQ; }
"="                     { return ASSN; }
">"                     { return GRTN; }
"<"                     { return LSTN; }
{VAR_NAME}              { yylval->strconst = strdup(yytext); return NAME; }
{int_const}             { yylval->intconst = atoi(yytext); return T_int; }
{string_const}          { yylval->strconst = strdup(yytext); return T_str; }
{whitespace}            { /* skip */ }
{comment}               { /* skip */ }

%%
