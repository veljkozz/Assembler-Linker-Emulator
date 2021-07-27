%option noyywrap nounput noinput

%{
#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include "bison.hpp"
int currLine = 1;
using namespace std;
%}

hex 0[xX][0-9a-fA-F]{1,4}
sym [a-zA-Z_][a-zA-Z_0-9]{0,29}
%%

[ \t]	  ; // ignore all whitespace
#.*\n {++currLine; return COMMENT; }; // ignore comments
\n		{ ++currLine; return T_NEWLINE;}
"."     {return T_DOT;}
","     {return T_COMMA;}
":"     {return T_COLON;}
"+"		{return T_PLUS;}
"-"		{return T_MINUS;}
"/"		{return T_DIVIDE;}
"("		{return PAR_L;}
")"		{return PAR_R;}
"$"     {return DOLLAR;}
"%"     {return PERCENT;}
"["     {return BRACKET_L;}
"]"     {return BRACKET_R;}
"*"     {return STAR;}
"exit"      {return T_QUIT;}
"quit"      {return T_QUIT;}
"global"    {return GLOBAL;}
"extern"    {return EXTERN;}
"section"   {return SECTION;}
"word"      {return WORD; }
"skip"      {return SKIP;}
"equ"       {return EQU;}
"end"       {return END;}
(r[0-7])      {yylval.ival = atoi(yytext+1); return REG;}
"sp"        {yylval.ival = 6; return REG;}
"pc"        {yylval.ival = 7; return REG;}
"psw"       {yylval.ival = 8; return REG;}
"halt"      {return HALT;}
"int"       {return INT;}
"iret"      {return IRET;}
"call"      {return CALL;}
"ret"       {return RET;}
"jmp"       {return JMP;}
"jeq"       {return JEQ;}
"jne"       {return JNE;}
"jgt"       {return JGT;}
"push"      {return PUSH;}
"pop"       {return POP;}
"xchg"      {return XCHG;}
"add"       {return ADD;}
"sub"       {return SUB;}
"mul"       {return MUL;}
"div"       {return DIV;}
"cmp"       {return CMP;}
"not"       {return NOT;}
"and"       {return AND;}
"or"        {return OR;}
"xor"       {return XOR;}
"test"      {return TEST;}
"shl"       {return SHL;}
"shr"       {return SHR;}
"ldr"       {return LDR;}
"str"       {return STR;}
{sym} {yylval.sval = strdup(yytext); return Symbol;}
[0-9]+      { yylval.ival = atoi(yytext); return NUMBER; }
{hex}       { yylval.ival = stoi(yytext, nullptr, 16); return NUMBER;}
%%

/*
"text"      {yylval.sval = strdup(yytext); return SectionName;} 
"rel"       {yylval.sval = strdup(yytext); return SectionName;}
"data"      {yylval.sval = strdup(yytext); return SectionName;}
"bss"       {yylval.sval = strdup(yytext); return SectionName;}
*/