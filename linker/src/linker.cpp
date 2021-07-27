#include <iostream>
#include <cstring>
#include "../inc/code.h"
using namespace std;

int main(int argc, char* argv[]){
    const char* fout = nullptr;
    vector<string> infiles;
    LinkOpt option = NO_OPTION;
    //cout << argc << endl;
    vector<Place> places;
    for(int i=1;i<argc;++i){
        
        string argvs = string(argv[i]);
        //cout << argvs << " ";
        if(strcmp(argv[i], "-o") == 0){
            if(i+1 == argc-1){
                cout << "error - command line arguments\n";
                exit(-1);
            }
            i++;
            fout = argv[i];
            
        }
        else if(strcmp(argv[i], "-hex") == 0){
            cout << "HEX" << endl;
            if(option != NO_OPTION) {
                cout << "error - more than one option\n";
                exit(-2);
            }
            option = HEX;
        }
        else if(strcmp(argv[i], "-linkable") == 0){
            if(option != NO_OPTION) {
                cout << "error - more than one option\n";
                exit(-2);
            }
            option = LINKABLE;
        }
        else if(argvs.substr(0,6) == "-place"){
            int monkeyPos = argvs.find('@');
            string sectionName = argvs.substr(7, monkeyPos-7);
            int addr = stoi("0x"+ argvs.substr(monkeyPos+1), nullptr, 16);
            addr = addr + (-addr % 8);
            places.push_back({sectionName, addr});
            //cout << "SecName:" << sectionName << " " << address << endl;
        }
        else if(argvs.substr(argvs.length()-2) == ".o"){
            infiles.push_back(argvs);
        }
    }
    if(option == NO_OPTION) {
        cout << "error - no option selected\n";
        exit(-3);
    }
    Code code(infiles, fout, option, places);
    code.loadFiles();
    cout << "Done loading files" << endl;
    code.merge();
    code.relloc();
    code.output();
}