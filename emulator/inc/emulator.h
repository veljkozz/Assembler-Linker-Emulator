#pragma once
#include <iostream>
#include <fstream>



using namespace std;




class Emulator{
public:
    Emulator();
    ~Emulator();
    static const int memsize = 1 << 16;
    static const int memReg = 0xff00;
    static const int nivt = 8;
    unsigned char* ivt;
    unsigned char* mem;
    unsigned short reg[8] = {0};
    unsigned short psw;

    int int_req = 0;
    //unsigned short& pc;
    //unsigned short& sp;
    void initMem();

    void psw_setz(short bit);
    void psw_seto(short bit);
    void psw_setc(short bit);
    void psw_setn(short bit);
    void psw_settr(short bit);
    void psw_settl(short bit);
    void psw_seti(short bit);
};
