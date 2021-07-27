#include "../inc/code.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <iomanip>
#include <algorithm>
Code::Code(vector<string> infiles, const char* fout, LinkOpt option, vector<Place> places){
    this->infiles = infiles;
    this->fout = fout;
    this->option = option;
    this->places = places;
}

void Code::loadFiles(){
    for(string fname: infiles){
        
        ifstream in(fname);
        cout << "Opened file " << fname << endl;
        string line;
        fileSymTab[fname] = map<string,Symbol>();
        while(getline(in, line)){
            //cout <<  "line:" << line << endl;
            if(line=="") continue;
            if(line.find("SymTab") != string::npos){
                getline(in, line);
                //cout << line << endl;
                while(1){
                    getline(in, line);
                    //cout << line << endl;
                    if(line=="") break;
                    char * cstr = new char [line.length()+1];
                    strcpy (cstr, line.c_str());
                    char* tok = strtok(cstr, "\t");
                    string name = string(tok);
                    tok = strtok(NULL, "\t");
                    int val = atoi(tok);
                    tok = strtok(NULL, "\t");
                    string section = string(tok);
                    tok = strtok(NULL, "\t");
                    int isGlobal = atoi(tok);
                    tok = strtok(NULL, "\t");
                    int size = atoi(tok);
                    tok = strtok(NULL, "\t");
                    Type tip = (Type)atoi(tok);
                    //cout << name << " " << val << " " << section << " " << isGlobal << endl;
                    Symbol s = {name, (short)val, section, (bool)isGlobal, size, tip, fname};
                    fileSymTab[fname][name] = s;
                    delete[] cstr;
                }
            }
            if(line.find("Rel") != string::npos){
                string section;
                relsPerFile[fname] = map<string, vector<Realloc>>();
                //cout << "Citanje rel" << endl;
                while(line != ""){
                    getline(in, line);
                    if(line=="") {
                        //cout << "Newline!\n";
                        break;
                    }
                    int dotpos = line.find(".");
                    if(dotpos != string::npos){
                        section = line.substr(dotpos+1);
                        //cout << "Rels for section:" << section << endl;
                        relsPerFile[fname][section] = vector<Realloc>();
                        getline(in, line);
                        continue;
                    }
                    

                    char * cstr = new char [line.length()+1];
                    strcpy (cstr, line.c_str());
                    char* tok = strtok(cstr, "\t");

                    string symbol = string(tok);
                    tok = strtok(NULL, "\t");
                    string sectiongarbage = string(tok);
                    tok = strtok(NULL, "\t");
                    int offs = atoi(tok);
                    tok = strtok(NULL, "\t");
                    int type = atoi(tok);
                    Realloc r = {symbol, offs, type};
                    //cout << "r " << symbol<< section << " " << offs << " " << type << endl;
                    relsPerFile[fname][section].push_back(r);
                }
                //cout << "Gotov sa rel" << endl;
            }
            if(line == "Code:"){
                sectionsPerFile[fname] = vector<Section>();
                while(getline(in, line)){
                    vector<char> secCode;
                    int pos = line.find(":");
                    string section = line.substr(0, pos);
                    int currpos = pos+1;
                    while(currpos+1 < line.length()){
                        string hexcode = "0x" + line.substr(currpos, 2);
                        //cout << hexcode << endl;
                        char byte = stoi(hexcode, nullptr, 16);
                        secCode.push_back(byte);
                        currpos+=3;
                    }
                    Section sect = {section, secCode, 0, fname, (int)secCode.size(), 0};
                    sectionsPerFile[fname].push_back(sect);
                    //showCode(secCode);
                }
            }
        }
        in.close();
        //this->symTable = fileSymTab[fname];
        //this->rels = relsPerFile[fname];
        //printSymTab();
        //printRel();

    }
}


// Possible errors
// Multiple definition
// Not defined - if option is -hex
// Overlapping sections
void Code::merge(){
    // merge sections from respective files and update their vals
    for(auto itr = sectionsPerFile.begin(); itr != sectionsPerFile.end(); itr++){
        for(Section s: itr->second){
            //cout << "Merging section " << s.name << " from file" << itr->first << endl;
            int pc;
            if(sectionTable.find(s.name) == sectionTable.end()){
                sectionTable[s.name] = s;
                pc=0;
            }else{
                pc = sectionTable[s.name].code.size();
                sectionTable[s.name].code.insert(
                    sectionTable[s.name].code.end(),
                    s.code.begin(), s.code.end()
                );
                sectionTable[s.name].size += s.code.size();
            }
            
            fileSymTab[s.file][s.name].val = pc;
            fileSymTab[s.file][s.name].file = s.file;
            //cout << ">>>>s.file=" << s.file << "<<<<<\n";
            //showCode(sectionTable[s.name].code);
        }
        this->symTable = fileSymTab[itr->first];
        printSymTab();
        this->symTable.clear();
        //cout << "--------------HM--------------" << endl;
    }
    //exit(-1);
    // check if there are overlapping sections - in case of -hex
    if(option == HEX){
        for(int i=0;i<places.size(); i++){
            for(int j=i+1;j<places.size(); j++){
                string seci = places[i].secName;
                string secj = places[j].secName;
                if((places[i].addr + sectionTable[seci].size > places[j].addr)
                || (places[j].addr + sectionTable[secj].size > places[i].addr)){
                    cout << "error: sections " << seci << " and " << secj << " are overlapping\n";
                    exit(-5);
                }
            }
        }
    }
    //cout << "-----MERGIN SYMBOLS--------" << endl;
    // merge symbols(except sections)
    // insert only global symbols
    for(auto itr = fileSymTab.begin(); itr != fileSymTab.end(); ++itr){
        for(auto symitr = itr->second.begin(); symitr != itr->second.end(); ++symitr){
            Symbol s = symitr->second;
            s.file = itr->first;
            if(s.tip == TSECTION)
                continue;
            if(s.isGlobal == false)
                continue;
            //cout << "Symbol " << s.name << " from file " << itr->first << endl;
            if(option == LINKABLE) {
                //cout << "old val=" << s.val;
                // increased by the offset of his section
                s.val += fileSymTab[itr->first][s.section].val;
                symitr->second.val += fileSymTab[itr->first][s.section].val;
                //cout << "  new val=" << s.val << endl;
            }
            if(symTable.find(s.name) == symTable.end()){
                symTable[s.name] = s;
            }
            else{
                if(symTable[s.name].section == "UNDEF"){
                    if(s.isGlobal == 1){
                        symTable[s.name] = s;
                    }
                }else{
                    cout << "error: multiple definitions of symbol " << s.name << endl;
                    exit(-6);
                }
            }
        }
    }
    // if option is hex all symbols must be defined
    if(option == HEX){
        for(auto itr = symTable.begin(); itr != symTable.end(); ++itr){
            if(itr->second.section == "UNDEF"){
                cout << "error: symbol " << itr->second.name << " undefined\n";
                exit(-8);
            }   
        }
    }

    //cout << "Good job so far!" << endl;
    //printSymTab();

}

void Code::relloc(){
    if(option == HEX){
        rellocHex();
        cout << "Code?" << endl;
        for(auto itr = sectionTable.begin(); itr != sectionTable.end(); ++itr){
            cout << itr->first << ": ";
            showCode(itr->second.code);
            cout << endl;
    }
    }else{
        rellocLink();
        printRel();
    }
    
}

bool cmpPlace(Place p1, Place p2){
    return p1.addr < p2.addr;
}

char upper(unsigned short w){
    return (char)((w >> 8) & 0xff);
}
char lower(unsigned short w){
    return (char)(w & 0x00ff);
}


void Code::rellocHex(){
    sort(places.begin(), places.end(), cmpPlace);
    // cout << "Sorted places" << endl;
    // for(Place p: places){
    //     cout << p.secName << " " << p.addr << ", ";
    // }
    // give every section a start place
    for(auto itr = sectionTable.begin(); itr != sectionTable.end(); ++itr){
        bool found = false;
        for(Place p: places){
            if(p.secName == itr->second.name){
                sectionTable[itr->first].start = p.addr;
                found = true;
                break;
            }
        }
        if(!found){
            Place nextp;
            if(places.size() > 0){
                Place lastP = places[places.size()-1];
                int nextAddr = lastP.addr + sectionTable[lastP.secName].size;
                nextp = {itr->first, nextAddr};
            }else{
                nextp = {itr->first, 0};
            }
            places.push_back(nextp);
            sectionTable[itr->first].start = nextp.addr;
        }
        Symbol secSymb = {itr->first, (short)sectionTable[itr->first].start, 
            itr->first, false, sectionTable[itr->first].size, TSECTION};
            symTable[itr->first] = secSymb;
    }
    //cout << endl;
    //cout << "=========SECTIONTABLE=========" << endl;
    for(auto itr = sectionTable.begin(); itr != sectionTable.end(); ++itr){
        //cout << itr->first << " : " << itr->second.start << endl;
    } 
    //cout << "==============================" << endl;
    for(auto itr=relsPerFile.begin(); itr != relsPerFile.end(); ++itr){
        string file = itr->first;
        for(auto itr2 = itr->second.begin(); itr2 != itr->second.end(); ++itr2){
            //cout << "======REL." << itr2->first << "======FILE-" << file << "=======\n";
            for(Realloc r: itr2->second){
                string section = itr2->first;
                
                // find value of section from this file
                int secOffs = fileSymTab[file][section].val;

                // value on location
                int addr = secOffs+r.offs;
                unsigned short high = (unsigned short)sectionTable[section].code[addr];
                unsigned short low = (unsigned short)sectionTable[section].code[addr+1];
                unsigned short val = (low&0xff) | (((high&0xff) << 8) & 0xff00);
                Symbol s = symTable[r.symbol];
                //if(s.name != "data") continue;
                // cout << "rel:" << r.symbol << " " << r.offs << " " << r.type << endl;
                // cout << "Addr: " << addr << ";" << "curr val:" << val << endl;
                // cout << s.name << "=" << s.val << ";" << s.section << "=" << sectionTable[s.section].start << endl;
                
                // cout << "Value of section " << section << " from file " << file << ": " << secOffs << endl;
                // cout << "Section " << section << " begins at " << sectionTable[section].start << endl;
                
                //this->symTable = fileSymTab[s.file];
                //printSymTab();
                //exit(1);
                
                if(r.type == 0){
                    // ABS
                    if(s.isGlobal == 0){
                        val += fileSymTab[file][s.name].val + sectionTable[s.section].start;
                    }else{
                        val += s.val;
                        val+= fileSymTab[s.file][s.section].val + sectionTable[s.section].start;
                    }
                }
                else if(r.type==1){
                    // PCREL
                    // pc + x = a
                    // x = a - pc
                    if(s.isGlobal == 0){
                        // na lokaciji su addend i offs
                        val += fileSymTab[file][s.name].val + sectionTable[s.section].start;
                    }else{
                        val += s.val;
                        val += fileSymTab[s.file][s.name].val + sectionTable[s.section].start;
                    }
                    // - pc
                    //cout << "pc=" << (sectionTable[section].start + addr) << endl;
                    val -= (sectionTable[section].start + addr);
                }
                //cout << "New val: " << val << endl<<endl;
                sectionTable[section].code[addr] = upper(val);
                sectionTable[section].code[addr+1] = lower(val);

                //showCode(sectionTable[section].code);
            }
        }
    }
}

void Code::rellocLink(){
    //cout << "===============RELLOC_LINKABLE===============" << endl;
    // add sections to symTable
    for(auto itr = sectionTable.begin(); itr != sectionTable.end(); ++itr){
        Symbol secSymb = {itr->first, 0, 
            itr->first, false, sectionTable[itr->first].size, TSECTION};
            symTable[itr->first] = secSymb;
    }

    for(auto itr=relsPerFile.begin(); itr != relsPerFile.end(); ++itr){
        for(auto itr2 = itr->second.begin(); itr2 != itr->second.end(); ++itr2){
            //cout << "Rel." << itr->first << endl;
            string section = itr2->first;
            if(rels.find(section) == rels.end())
                rels[section] = vector<Realloc>();
            for(Realloc r: itr2->second){
                
                string file = itr->first;
                // find value of section from this file
                int secOffs = fileSymTab[file][section].val;
                int addr = secOffs+r.offs;
                unsigned short high = (unsigned short)sectionTable[section].code[addr];
                unsigned short low = (unsigned short)sectionTable[section].code[addr+1];
                unsigned short val = (low&0xff) | (((high&0xff) << 8) & 0xff00);
                Symbol s = symTable[r.symbol];
                //cout << "r:" << r.symbol << "," << r.offs << "," << r.type << endl;
                //cout << "Addr: " << addr << ";" << "curr val:" << val << endl;
                r.offs += secOffs;
                if(r.type == 0){
                    // ABS
                    if(s.isGlobal == 0){
                        //cout << "AbsLocal" << endl;
                        rels[section].push_back(r);
                        // do nothing? cant know position of section
                    }else{
                        //cout << "AbsGlobal" << endl;
                        // add symbol offs to location if it is defined
                        //cout << "Symbol is in section " << s.section << endl;
                        if(s.section != "UNDEF"){
                            
                            val += fileSymTab[s.file][s.name].val+fileSymTab[s.file][s.section].val;
                            r.symbol = s.section;
                            rels[section].push_back(r);
                        }
                    }
                }else{
                    // PCREL
                    // pc + x = a
                    // x = a - pc
                    // x = sec(a) + offs(a) - pc
                    if(s.isGlobal == 0){
                        //cout << "PcLocal" << endl;
                        rels[section].push_back(r);
                        // do nothing? cant know position of section
                    }else{
                        // if symbol is in same section as rel
                        // we can relocate it
                        //cout << "PcGlobal" << endl;
                        if(symTable[s.name].section == section){
                            val += symTable[s.name].val;
                             // - pc
                            //cout << "pc=" << addr << endl;
                            val -= addr;
                        }
                        // if it isnt in same section, 
                        else{
                            if(symTable[s.name].section != "UNDEF"){
                                val += symTable[s.name].val + fileSymTab[s.file][s.section].val;
                                r.symbol = s.section;
                            }
                             rels[section].push_back(r);
                        }
                    }
                   
                }

                //cout << "New val: " << val << endl;
                sectionTable[section].code[addr] = upper(val);
                sectionTable[section].code[addr+1] = lower(val);

                //showCode(sectionTable[section].code);
            }
        }
    }   

}

void Code::output(){
    if(this->fout != nullptr){
        ofstream out(fout);
        
        int cnt = 0;
        out << hex << setfill('0');
        if(option == HEX){
            for(Place p: places){
                cnt = sectionTable[p.secName].start;
                for(unsigned char byte: sectionTable[p.secName].code){
                    if(cnt % 8 == 0){
                        out << setw(4)  << cnt << ": ";
                    }
                    out << setw(2) << (int)byte << " ";
                    cnt++;
                    if(cnt%8 == 0)
                        out << endl;
                }
            }
        }else{
            cout << dec;
            out << dec;
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
                    out << r.symbol << "\t" << itr->first << "\t" << r.offs << "\t" << r.type << endl;
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
        }
        out.close();
    }
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
        cout << "Symbol\tOffs\tType\n";
        for(Realloc r: itr->second){
            cout << r.symbol << "\t" << r.offs << "\t" << r.type << endl;
        }
    }
}


void Code::showCode(vector<char> code){
    cout << "Code: \n";
    vector<char> sectionCode;
    
    for(unsigned char b: code){
        cout << setw(2) << setfill('0') << hex << (int)b << " ";
    }
    cout << endl;
}