#include "../inc/emulator.h"
#include "../inc/timer.h"
#include <ctime>
Timer::Timer(Emulator* e) : e(e) {
    t = clock() * 1000 / CLOCKS_PER_SEC;
    cfg = &e->mem[0xff11];
}

void Timer::tick(){
    int period;
    //*cfg = 7;
    switch(*cfg){
        case 0:
        period = 500;break;
        case 1:
        period = 1000; break;
        case 2:
        period = 1500; break;
        case 3:
        period = 2000; break;
        case 4:
        period = 5000; break;
        case 5:
        period = 10000; break;
        case 6:
        period = 30000; break;
        case 7:
        period = 60000; break;
    }
    unsigned long newt = clock() * 1000 / CLOCKS_PER_SEC;
    if(newt-t >= period){
        //cout << "Timer!" << endl;
        t = newt;
        e->int_req |= 1 << 2;
    }
}