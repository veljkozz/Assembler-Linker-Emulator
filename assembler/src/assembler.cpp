#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "code.h"
#include <cstring>
using namespace std;

extern FILE* yyin;
extern int yyparse();
extern int yylex_destroy(void);
extern int currLine;
bool firstPass;
bool done = false;
Code code;
int main(int argc, char* argv[]) {
    const char* outf = nullptr;
    if(argc < 2){
        cout << "Fajl?" << endl;
        return -1;
    }
	else{
        if(argc == 4 && strcmp(argv[1], "-o") == 0){
            outf = argv[2];
            yyin = fopen(argv[3], "r");
        }
        else
            yyin = fopen(argv[2], "r");

        if(yyin == nullptr){
            cout << "File " << argv[3] << " does not exist" << endl;
            return -100;
        }
    }
    firstPass = true;
	do {
		yyparse();
	} while(!feof(yyin));
    fseek(yyin, 0, SEEK_SET);
    currLine = 0;
    //cout << "Second pass?\n";
    firstPass = false;
    do {
		yyparse();
	} while(!feof(yyin));
    code.printSymTab();
    code.printRel();
    code.showCode();
    if(outf != nullptr){
        code.writeFile(outf);
        // create object file
    }
	return 0;
}