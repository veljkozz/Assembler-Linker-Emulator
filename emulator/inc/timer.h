#pragma once
class Emulator;

class Timer{
    unsigned long t;
    static const unsigned short cfg_loc = 0xff10;
    Emulator *e;
    unsigned char* cfg;
public:
    Timer(Emulator*);
    void tick();
};
