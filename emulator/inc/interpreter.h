#pragma once
#include <set>
#include <vector>
#include <map>
class Emulator;
class Terminal;
class Timer;
using namespace std;

enum ADDR{ 
	IMMED = 0, REGDIR = 1, REGPLUS = 5, REGIND = 2, 
    REGINDPLUS = 3, MEM = 4};

class Interpreter{
private:
    static map<unsigned char, string> instrmap;
    const char* filename;
    bool end;
    Emulator* em;
    Terminal* terminal;
    Timer* timer;
    set<unsigned short> instr0 = {0x00, 0x20, 0x40};
    set<unsigned short> instr1 = {0x10, 0x80};
    set<unsigned short> instr2 = {0x60, 0x70, 0x71, 0x72, 0x73, 0x74, 0x81, 0x82, 0x83, 0x84, 0x90, 0x91};
    set<unsigned short> ld_st = {0xA0, 0xB0};
    set<unsigned short> instr_jmp = {0x30, 0x50, 0x51, 0x52, 0x53};
    void exec(int instrAddr, unsigned char opcode, unsigned char regsdescr = 0, 
    ADDR addressing = IMMED,  unsigned char up = 0, unsigned short data = 0);

    unsigned short readWBig(unsigned short);
    unsigned short readWSmall(unsigned short);
    unsigned short readW(unsigned short addr);
    void storeW(unsigned short, unsigned short);
    void storeWBig(unsigned short, unsigned short);

    void halt();
    void iret();
    void ret();
    void pop(short);
    void push(short);
    void interrupt(short);
    void ldr(short, short, short, unsigned char, unsigned short);
    void str(short, short, short, unsigned char, unsigned short);
    void jmp(short regD, short regS, short addressing, unsigned char up, unsigned short data);

    unsigned short fetchOperandLd(short regS, short addressing, unsigned char up, unsigned short data);

    void xcgh(short regD, short regS);
    void add(short regD, short regS);
    void sub(short regD, short regS);
    void mul(short regD, short regS);
    void div(short regD, short regS);
    void cmp(short regD, short regS);
    
    void Not(short regD);
    void And(short regD, short regS);
    void Or(short regD, short regS);
    void Xor(short regD, short regS);
    void test(short regD, short regS);

    void shl(short regD, short regS);
    void shr(short regD, short regS);

    bool bitZ(){return ((em->psw & 0x1) == 1); }
    bool bitO(){return ((em->psw & 0x2) == 0x2); }
    bool bitC(){return ((em->psw & 0x4) == 0x4); }
    bool bitN(){return ((em->psw & 0x8) == 0x8); }

    void int_i(int i);
    void interrupts();
public:
    void loadMem();
    Interpreter(const char *file, Emulator*, Terminal*, Timer*);
    void readexec();
};