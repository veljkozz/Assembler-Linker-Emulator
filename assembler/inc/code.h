#pragma once
#include <vector>
#include <map>

using namespace std;

enum Type{ NO_TYPE=0, TOBJECT, TSECTION};

struct Symbol{
    string name;
    short val;
    string section;
    int size;
    bool isGlobal;
    Type tip;
    Symbol(){
        size = 0;
        section = "UNDEF";
        val = 0;
        tip = NO_TYPE;
    }
    Symbol(string name,short val = 0,string section = "UNDEF",int size = 0, bool isGlobal = false, Type tip = TOBJECT){
        this->name = name;
        this->val = val;
        this->section = section;
        this->size = size;
        this->isGlobal = isGlobal;
        this->tip = tip;
    }
};

struct Section{
    string name;
    vector<char> code;
    int flags;      // not used anywhere yet
};


struct Realloc{
    string symbol;
    string section;
    int offs;
    int type;
};

class Code{
public:
    map<string, Symbol> symTable;
    //vector<Section> sections;
    map<string, Section> sectionTable;
    vector<char> currCode;
    map<string, vector<Realloc>> rels;
    string currSection;
    int currSecSize;
    void addByte(char b);
    void addWord(unsigned short w);
    Code();
    void showCode();
    
    void addSymbol(string name, string section, unsigned short val, bool global, Type tip = TOBJECT);
    void setSymSize(string name, int size);
    bool symExists(string name);
    void printSymTab();
    
    void addRealloc(string sym, string section, int pc, int relType);
    void printRel();

    void addSection();

    void writeFile(const char* filename);
    static unsigned short reverse(unsigned short word);
    static char upper(unsigned short word);
    static char lower(unsigned short word);
};