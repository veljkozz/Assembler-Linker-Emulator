#pragma once
#include "emulator.h"

class Terminal{
    
    static int descriptor;
    Emulator* e;
    
    void print();

    static void restore();
    
public:
    static const unsigned short out = 0xff00;
    static const unsigned short in = 0xff02;
    Terminal(Emulator*);
    ~Terminal();
    static void signal(int num);
    void readInput();
};