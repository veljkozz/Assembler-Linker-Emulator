#include "../inc/code.h"
#include <iostream>
#include <iomanip>
#include <fstream>
extern int currLine;
Code::Code(){
    this->currSection = "UNDEF";
    this->currSecSize = 0;
    Symbol sym = {"UNDEF", 0, "UNDEF", 0, true, TSECTION};
    symTable[sym.name] = sym;
}

void Code::addSymbol(string name, string section, unsigned short val, bool global, Type tip){
    if(symTable.find(name) != symTable.end()){
        cout << "error at line " << currLine << ": Symbol " << name << " already defined" << endl;
        exit(-200);
    }
    Symbol sym = Symbol();
    sym.name = name; sym.val = val;
    sym.section = section;
    sym.isGlobal = global;
    sym.tip = tip;
    symTable[name] = sym;//Symbol(name, val, section, global);
}

void Code::setSymSize(string name, int size){
    symTable[name].size = size;
}

void Code::addByte(char b){
    currCode.push_back(b);
}

void Code::addWord(unsigned short w){
    currCode.push_back(upper(w));
    currCode.push_back(lower(w));
}

void Code::addSection(){
    Section sec;
    string name = currSection;
    sec.name = name;
    sec.code = currCode;
    sectionTable[name] = sec;
    currCode.clear();
}

unsigned short Code::reverse(unsigned short word){
    return ((word << 8) & 0xff00) | ((word >> 8)& 0x00ff); 
}

char Code::upper(unsigned short w){
    return (char)((w >> 8) & 0xff);
}
char Code::lower(unsigned short w){
    return (char)(w & 0x00ff);
}

void Code::printSymTab(){
    cout << "----------------------------------" << endl;
    cout << "SymTab: \n";
    cout << "Name\tVal\tSection\tGlobal?\tSize\tTip\n";
    for(auto itr = symTable.begin(); itr != symTable.end(); ++itr){
        Symbol sym = itr->second;
        cout << sym.name << "\t" << sym.val << "\t" << sym.section <<
         "\t" << sym.isGlobal << "\t" << sym.size <<  "\t" << sym.tip << endl;
    }
    cout << "----------------------------------" << endl;
}

void Code::printRel(){
    
    for(auto itr = rels.begin(); itr != rels.end(); itr++){
        cout << "rel."<< itr->first << endl;
        cout << "Symbol\tSection\tOffs\tType\n";
        for(Realloc r: itr->second){
            cout << r.symbol << "\t" << r.section << "\t" << r.offs << "\t" << r.type << endl;
        }
    }
}

void Code::showCode(){
    cout << "Code: \n";
    vector<char> sectionCode;
    for(auto itr = sectionTable.begin(); itr != sectionTable.end(); ++itr){
        cout << itr->first << ":";
        for(unsigned char b: itr->second.code){
            cout << setw(2) << setfill('0') << hex << (int)b << " ";
        }
        cout << endl;
    }
}


bool Code::symExists(string name){
    return (symTable.find(name) != symTable.end());
}

void Code::addRealloc(string sym, string section, int pc, int relType){
    Symbol s;
    if(symExists(sym)){
        s = symTable[sym];
        if(s.section == "ABS"){
            addWord(s.val);
            return;
        }
    }
    else{
        cout << "Symbol " << sym << " not defined on line " << currLine << "!!!" << endl;
        addSymbol(sym, "UNDEF", pc, true);
        s = symTable[sym];
    }
    Realloc rel = {sym, section, pc, relType};
    if(relType == 0){
        // ABS
        if(s.isGlobal){
            addWord(0);
        }
        else{
            addWord(s.val);
            rel.symbol = s.section;
        }
    }
    else if(relType == 1){
        // PCREL 
        // offs - addend = pc
        // pc+x=a
        // x = a - pc
        // x = sec(a) + offs(a) - pc + addend
        if(s.isGlobal){
            // just addend
            addWord(-2);
        }else{
            if(s.section == section){
                cout << "S.VAL-" << s.val << "  PC-" << pc << endl;
                addWord(s.val - 2);
                return;
            }
            else{
                // addend plus offs, rel symbol is now the section
                addWord(s.val - 2);
                rel.symbol = s.section;
            }
        }
    }
    if(rels.find(section) == rels.end()) rels[section] = vector<Realloc>();
    rels[section].push_back(rel);
}

void Code::writeFile(const char* filename){
    ofstream out;
    out.open(filename);
    out << "\n";
    out << "SymTab: \n";
    out << "Name\tVal\tSection\tGlobal?\tSize\tTip\n";
    for(auto itr = symTable.begin(); itr != symTable.end(); ++itr){
        Symbol sym = itr->second;
        out << sym.name << "\t" << sym.val << "\t" << sym.section <<
         "\t" << sym.isGlobal << "\t" << sym.size <<  "\t" << sym.tip << endl;
    }
    out << "\n";
    out << "Rel: \n";
    for(auto itr = rels.begin(); itr != rels.end(); itr++){
        out << "rel."<< itr->first << endl;
        out << "Symbol\tSection\tOffs\tType\n";
        for(Realloc r: itr->second){
            out << r.symbol << "\t" << r.section << "\t" << r.offs << "\t" << r.type << endl;
        }
    }
    out << "\n";
    out << "Code:\n";
    for(auto itr = sectionTable.begin(); itr != sectionTable.end(); ++itr){
        out << itr->first << ":";
        for(unsigned char b: itr->second.code){
            out << setw(2) << setfill('0') << hex << (int)b << " ";
        }
        out << endl;
    }
    cout << "\n";
    out.close();
}