%{
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include "code.h"
using namespace std;

// Declare stuff from Flex that Bison needs to know about:
extern int yylex();
extern int yyparse();
extern FILE *yyin;
extern Code code;
void yyerror(const char *s);


void yyerror(const char* s);

struct sym_lit{
	bool isSymbol;
	string symbol;
	int litval;
};
using namespace std;

vector<string> symList;
vector<sym_lit> symLitList;
extern int currLine;
unsigned short pc=0;

extern bool done;
extern bool firstPass;
enum ADDR{ 
	IMMED = 0, REGDIR = 1, REGPLUS = 5, REGIND = 2, REGINDPLUS = 3, MEM = 4};
struct operand{
	ADDR addressing;
	int reg;
	unsigned short val;
	int up;		// using this in PUSH and POP
	bool isSymbol;
	int relType;
	string symbol;
	operand(){
		isSymbol = false;
	}
};
operand currOp;

%}


%union {
	int ival;
	float fval;
	char *sval;
	long long lval;
}

// Assembler directives
%token GLOBAL EXTERN SECTION WORD SKIP EQU END
%token<sval> Symbol 
%token<ival> NUMBER
%token<ival> REG
%token T_PLUS T_MINUS
%token T_DOT T_COMMA T_COLON COMMENT WHATEVER DOLLAR 
STAR PERCENT BRACKET_L BRACKET_R PAR_L PAR_R
%token T_NEWLINE T_QUIT
// instr0
%token HALT RET IRET
// jmp instr
%token JMP JEQ JNE JGT CALL 
// instr1
%token PUSH POP INT NOT
// instr2
%token XCHG ADD SUB MUL DIV CMP AND OR XOR TEST SHL SHR
%token LDR STR

%left T_PLUS T_MINUS
%left T_MULTIPLY T_DIVIDE
%type Instr
%type Directive 
%type sym_list
%type instr0
%type instr1 
%type instr2 
%type instr_jmp
%type<ival> Literal

%start program

%%

program:
	line 
	| line program
;

line: T_NEWLINE
	| COMMENT
	| Label T_NEWLINE
	| Label COMMENT
	| Label instr_directive T_NEWLINE
	| Label instr_directive COMMENT
	| instr_directive T_NEWLINE
	| instr_directive COMMENT
;

Label:
	Symbol T_COLON	{ 
		// add to symbol table!
		if(firstPass)
			code.addSymbol(string($1), code.currSection, pc, false);
		}
;

instr_directive: Instr	{/*cout << "pc=" << pc << endl;*/}
	| T_DOT Directive
;

Instr:
	instr0 { pc+=1; }
	| instr1 REG 
	{
		pc+=2;
		if(!firstPass){
			code.addByte(0x0f | ($2 << 4));
		}
	}
	| instr2 REG  T_COMMA REG 
	{
		pc+=2;
		if(!firstPass){
			code.addByte($4 | ($2 << 4));
		}
	}
	| instr_jmp Operand_jmp
	{
		pc+=3;
		if(!firstPass){
			code.addByte(currOp.reg | 0xf0);
			code.addByte(currOp.addressing | (currOp.up << 4));
			if(currOp.addressing == IMMED || currOp.addressing == REGPLUS ||
			currOp.addressing == REGINDPLUS || currOp.addressing == MEM){
				if(currOp.isSymbol == false)
					code.addWord(currOp.val);
				else{
					code.addRealloc(currOp.symbol, code.currSection, (unsigned char)pc,
					 currOp.relType);
				}
				
			}
		}
		if(currOp.addressing == IMMED || currOp.addressing == REGPLUS ||
			currOp.addressing == REGINDPLUS || currOp.addressing == MEM)
			pc+=2;
	}
	| LD_ST REG T_COMMA Operand_mem
	{
		pc+=3;
		if(!firstPass){
			//cout << (currOp.reg | ($2 << 4)) << endl;
			code.addByte(currOp.reg | ($2 << 4));
			code.addByte(currOp.addressing | (currOp.up << 4));
			if(currOp.addressing == IMMED || currOp.addressing == REGPLUS ||
			currOp.addressing == REGINDPLUS || currOp.addressing == MEM){
				if(currOp.isSymbol == false)
					code.addWord(currOp.val);
				else{
					code.addRealloc(currOp.symbol, code.currSection, (unsigned char)pc,
					 currOp.relType);
				}
				
			}
		}
		if(currOp.addressing == IMMED || currOp.addressing == REGPLUS ||
			currOp.addressing == REGINDPLUS || currOp.addressing == MEM)
			pc+=2;
	}
	| PUSH_POP REG
	{
		pc+=3;
		// sp = r6
		if(!firstPass){
			code.addByte(0x6 | ($2 << 4));
			code.addByte(REGIND | (currOp.up << 4));
		}
		

	}
;

instr0: HALT 	{if(!firstPass)code.addByte(0x00);}
	| IRET		{if(!firstPass)code.addByte(0x20);}
	| RET 		{if(!firstPass)code.addByte(0x40);}
;
instr1:
	  INT		{if(!firstPass)code.addByte(0x10);}
	| NOT		{if(!firstPass)code.addByte(0x80);}
;
PUSH_POP:
	  PUSH		{if(!firstPass) {code.addByte(0xB0); currOp.up = 0x1;}}
	| POP		{if(!firstPass) {code.addByte(0xA0); currOp.up = 0x4;}}
;
LD_ST:
	 LDR		{if(!firstPass) code.addByte(0xA0);}
	|STR		{if(!firstPass) code.addByte(0xB0);}
;
instr2:
	  XCHG		{if(!firstPass)code.addByte(0x60);}
	| ADD		{if(!firstPass)code.addByte(0x70);}
	| SUB		{if(!firstPass)code.addByte(0x71);}
	| MUL		{if(!firstPass)code.addByte(0x72);}
	| DIV		{if(!firstPass)code.addByte(0x73);}
	| CMP		{if(!firstPass)code.addByte(0x74);}
	| AND		{if(!firstPass)code.addByte(0x81);}
	| OR		{if(!firstPass)code.addByte(0x82);}
	| XOR		{if(!firstPass)code.addByte(0x83);}
	| TEST		{if(!firstPass)code.addByte(0x84);}
	| SHL		{if(!firstPass)code.addByte(0x90);}
	| SHR		{if(!firstPass)code.addByte(0x91);}
;
instr_jmp:
	CALL		{if(!firstPass)code.addByte(0x30);}
	| JMP		{if(!firstPass)code.addByte(0x50);}
	| JEQ		{if(!firstPass)code.addByte(0x51);}
	| JNE		{if(!firstPass)code.addByte(0x52);}
	| JGT		{if(!firstPass)code.addByte(0x53);}
;
Operand_jmp:
	  NUMBER
	{
		currOp.addressing = IMMED;
		currOp.val = $1;
		currOp.reg = 0xf;
		currOp.isSymbol = false;
	}
	| Symbol
	{
		currOp.addressing = IMMED;
		currOp.isSymbol = true;
		currOp.symbol = $1;
		currOp.reg = 0xf;
		currOp.relType = 0;
	}
	| PERCENT Symbol	
	{
		/*pcrel*/ 
		currOp.addressing = REGPLUS;
		currOp.reg = 7;
		currOp.symbol = $2;
		currOp.isSymbol = true;
		currOp.relType = 1;
	}
	| STAR NUMBER
	{
		currOp.addressing = MEM;
		currOp.val = $2;
		currOp.reg = 0xf;
		currOp.isSymbol = false;
	}
	| STAR Symbol
	{
		currOp.addressing = MEM;
		currOp.reg = 0xf;
		currOp.symbol = $2;
		currOp.isSymbol = true;
		currOp.relType = 0;
		// ADD RELOC
	}
	| STAR REG{
		currOp.addressing = REGDIR;
		currOp.reg = $2;
		currOp.val = 0xff;
		currOp.isSymbol = false;
	}
	| STAR BRACKET_L REG BRACKET_R
	{
		currOp.addressing = REGIND;
		currOp.reg = $3;
		currOp.val = 0xff;
		currOp.up = 0;
		currOp.isSymbol = false;
	}
	| STAR BRACKET_L REG T_PLUS NUMBER BRACKET_R 
	{
		currOp.addressing = REGINDPLUS;
		currOp.reg = $3;
		currOp.val = $5;
		currOp.up = 0;
		currOp.isSymbol = false;
	}
	| STAR BRACKET_L REG T_PLUS Symbol BRACKET_R	
	{
		/*[reg+simbol]*/
		currOp.addressing = REGINDPLUS;
		currOp.reg = $3;
		currOp.isSymbol = true;
		currOp.up = 0;
		currOp.symbol = $5;
		currOp.relType = 0;
	}
;
Operand_mem:
	  DOLLAR NUMBER		
	{
		/*
		enum ADDR{ 
		IMMED = 0, REGDIR = 1, REGPLUS = 5, REGIND = 2, 
		REGINDPLUS = 3, MEM = 4}
		struct operand{
			ADDR addressing;
			int reg;
			unsigned short val;
			int up;		// using this in PUSH and POP
		};
		*/
		currOp.addressing = IMMED;
		currOp.val = $2;
		currOp.reg = 0xf;
		currOp.up = 0;
		currOp.isSymbol = false;
	}
	| DOLLAR Symbol
	{
		currOp.addressing = IMMED;
		currOp.symbol = $2;
		currOp.relType = 0;
		currOp.reg = 0xf;
		currOp.up = 0;
	
	}
	| NUMBER
	{
		currOp.addressing = MEM;
		currOp.val = $1;
		currOp.reg = 0xf;
		currOp.isSymbol = false;
	
	}
	| Symbol
	{
		currOp.addressing = MEM;
		currOp.reg = 0xf;
		currOp.isSymbol = true;
		currOp.symbol = $1;
		currOp.relType = 0;
	}
	| PERCENT Symbol	
	{
		currOp.addressing = REGINDPLUS;
		currOp.reg = 7;
		currOp.up = 0;
		currOp.isSymbol = true;
		currOp.relType = 1;
		currOp.symbol = $2;
	}
	| REG
	{
		currOp.addressing = REGDIR;
		currOp.reg = $1;
		currOp.isSymbol = false;
	}
	| BRACKET_L REG BRACKET_R
	{
		currOp.addressing = REGIND;
		currOp.reg = $2;
		currOp.up = 0;
		currOp.isSymbol = false;
	}
	| BRACKET_L REG T_PLUS Literal BRACKET_R 
	{
		currOp.addressing = REGINDPLUS;
		currOp.up = 0;
		currOp.reg = $2;
		currOp.val = $4;
		currOp.isSymbol = false;
	}	
	| BRACKET_L REG T_PLUS Symbol BRACKET_R	
	{
		currOp.addressing = REGINDPLUS;
		currOp.up = 0;
		currOp.reg = $2;
		currOp.isSymbol = true;
		currOp.symbol = $4;
		currOp.relType = 0;
	}
;

Directive:
	GLOBAL sym_list 		
	{ 
		if(!firstPass){
			for(std::string sym: symList){
				code.symTable[sym].isGlobal = true;
			}
		}
	}
|	EXTERN sym_list 		
	{
		if(!firstPass){
			for(std::string sym: symList){
				if(!code.symExists(sym)) 
					code.addSymbol(sym, "UNDEF", 0, true);
			}
		}
	}
|	SECTION Symbol	
	{ 	
		if(firstPass){
			//cout << code.currSection << " size:" << pc << endl;
			code.setSymSize(code.currSection, pc);
			code.currSection = $2;
			pc = 0;
			code.addSymbol($2, $2, 0, false, TSECTION);
		}else{
			// STORE SECTION CONTENT IN MEMORY
			code.addSection();
			code.currSection = $2;
			pc = 0;
		}
		//printf("Section %s\n", $2);
		
	}
|	WORD sym_lit_list  	
	{ 
	for(sym_lit it: symLitList){
		if(!firstPass){
			if(it.isSymbol){
				code.addRealloc(it.symbol, code.currSection, pc, 0);
			}
			else{
				unsigned short w = (unsigned short)it.litval;
				code.addWord(w);
				}
		}
		pc+=2;
	}
	}
|	SKIP Literal 		
{
	pc+=$2;
	if(!firstPass)
		for(int i=0;i<$2; i++){
			code.addByte(0);
		}
}
|	EQU Symbol T_COMMA Literal 
	{
		if(firstPass) code.addSymbol($2, "ABS", $4, false);
	}
|	END 
	{
		if(firstPass)	code.setSymSize(code.currSection, pc);
		else code.addSection();
 		YYACCEPT;
	}
;

sym_list: Symbol					{symList.clear(); symList.push_back(string($1)); }
	| sym_list T_COMMA Symbol	{ symList.push_back(string($3));}
;
Literal:
	NUMBER { $$ = $1; }
	| T_PLUS NUMBER { $$ = $2;}
	| T_MINUS NUMBER {$$ = -1* $2;}
;
sym_lit_list: Symbol		{
		sym_lit sym = {true, $1};
		symLitList.clear(); 
		symLitList.push_back(sym); }
	| Literal			
	{
		sym_lit lit = {false, "", $1};
		lit.litval = $1;
		symLitList.clear();
		symLitList.push_back(lit);
	}
	| sym_lit_list T_COMMA Symbol {
		sym_lit sym = {true, $3, 0};
		symLitList.push_back(sym); 
	}
	| sym_lit_list T_COMMA Literal {
		sym_lit lit = {false, "bla", $3};
		lit.litval = $3;
		symLitList.push_back(lit);
	} 
;



%%


void yyerror(const char* s) {
	fprintf(stderr, "Parse error on line %d: %s\n", currLine, s);
	exit(1);
}
