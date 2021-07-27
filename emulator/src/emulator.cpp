#include "../inc/emulator.h"
#include "../inc/interpreter.h"
#include "../inc/terminal.h"
#include "../inc/timer.h"

int main(int argc, const char* argv[]){
    if(argc != 2){
        cout  << "error: invalid command line arguments\n";
        exit(-1);
    }    
    cout << argv[0] << endl;
    cout << argv[1] << endl;
    const char *fin = argv[1];
    string fname = string(fin);

    // FILE* fptr= fopen(fin, "r");
    // if(fptr == nullptr) cout << "OTVORENI JEBENI FAJL??!" << endl;
    // else {
    //     cout << "MENI SAMO NIJE JASNO???" << endl;
    //     fclose(fptr);
    // }
    ifstream in(fname);
    string line;
    getline(in, line);
    cout << "First line:" << line << endl;
    in.close();
    Emulator e;
    Terminal terminal(&e);
    Timer timer(&e);
    Interpreter interpreter(fin, &e, &terminal, &timer);
    cout << "loading . . . " << endl;
    interpreter.loadMem();
    cout << "executing. . . " << endl;
    interpreter.readexec();
    //cout << "Ja ne kapiram sta puca" << endl;

}

Emulator::Emulator(){
    initMem();
}
void Emulator::initMem(){
    mem = new unsigned char[65536];
    ivt = (unsigned char*)mem;
    psw = 0x0;
    //pc = reg[7];
    //sp = reg[6];
}

Emulator::~Emulator(){
    ivt = 0;
    delete[] mem;
}
void Emulator::psw_setz(short bit){
    psw = (psw & 0xfffe) | bit;
}

void Emulator::psw_seto(short bit){
    psw = (psw & 0xfffd) | (bit << 1);
}

void Emulator::psw_setc(short bit){
    psw = (psw & 0xfffb) | (bit << 2);
}

void Emulator::psw_setn(short bit){
    psw = (psw & 0xfff7) | (bit << 3);
}

void Emulator::psw_settr(short bit){
    psw = (psw &  0xdfff) | (bit << 13);
}

void Emulator::psw_settl(short bit){
    psw = (psw & 0xbfff) | (bit << 14);
}

void Emulator::psw_seti(short bit){
    psw = (psw & 0x7fff) | (bit << 15);
}