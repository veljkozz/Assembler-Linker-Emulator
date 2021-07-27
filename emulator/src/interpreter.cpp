#include "../inc/emulator.h"
#include "../inc/interpreter.h"
#include "../inc/timer.h"
#include "../inc/terminal.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#define rpc 7
#define rsp 6
#define rpsw 8

#define pc em->reg[7]
#define sp em->reg[6]
map<unsigned char, string> Interpreter::instrmap = {
        {0x00, "HALT"},
        {0x20, "IRET"},
        {0x40, "RET"},
        {0x10, "INT"},
        {0x80, "NOT"},
        {0xA0, "LDR"},
        {0xB0, "STR"},
        {0x60,"XCHG"},
        {0x70, "ADD"},
        {0x71, "SUB"},
        {0x72, "MUL"},
        {0x73, "DIV"},
        {0x74, "CMP"},
        {0x81, "AND"},
        {0X82, "OR"},
        {0X83, "XOR"},
        {0X84, "TEST"},
        {0X90, "SHL"},
        {0X91, "SHR"},
        {0X30, "CALL"},
        {0X50, "JMP"},
        {0X51, "JEQ"},
        {0X52, "JNE"},
        {0X53, "JGT"}
};

Interpreter::Interpreter(const char *file, Emulator* e, Terminal* term, Timer* timer){
    this->filename = file;
    this->em = e;
    this->terminal = term;
    this->timer = timer;
    sp = 0xF000;
}

void Interpreter::loadMem(){
    ifstream in(filename);
    string line;
    while(getline(in, line)){
        cout << line.substr(0,4) << ": ";
        int addr = stoi(line.substr(0,4), 0, 16);
        istringstream is(line.substr(5));
        //unsigned shor valhigh;
        //unsigned char vallow;
        unsigned short val;
        is >> hex;
        while(is >> val){
            //is >> vallow;
            //val = vallow + (valhigh << 4);
            em->mem[addr] = (unsigned char)val;
            cout << hex << setw(2) << setfill('0') << val <<  " ";
            addr++;
        }
        cout << endl;
    }
    in.close();
}

void Interpreter::readexec(){
    //FILE* fin = fopen(filename, "r");
    //if(fin == nullptr) cout << "File??" << endl;
    //else fclose(fin);

    ifstream in;
    in.open(filename);
    string line;
    pc = ((unsigned short)em->mem[0]  << 8) + (unsigned char)em->mem[1];
    //getline(in, line);
    //cout << "First line:" << line << endl;
    //while(getline(in, line)) cout << line << endl;
    //pc = stoi(line.substr(0,4), nullptr, 16);
    end = false;
    while(!end){
        
        //cout << "pc=" << pc << ": ";
        int instrAddr = pc;
        unsigned char tmp;
        unsigned char opcode, regdescr, addressing, up;
        unsigned short data;
        unsigned char datalow, datahigh;
        opcode = em->mem[pc++];
        //cout << "(opcode:" << (short)opcode << ") ";
        for(int i=0;i<32000;i++){
            for(int j=0;j<32;j++){
                int k = 0;
            }
        }
        if(instr0.find(opcode) != instr0.end()){
            exec(instrAddr, opcode);
        }
        else{
            regdescr = em->mem[pc++];
            if(instr1.find(opcode) != instr1.end()){
                //cout << " instr1 ";
                exec(instrAddr, opcode, regdescr);
            }
            else if(instr2.find(opcode) != instr2.end()){
                //cout << " instr2 ";
                exec(instrAddr, opcode, regdescr);
            }
            else{
                addressing = em->mem[pc++];
                up = (addressing >> 4) & 0xf;
                addressing = addressing & 0xf;
                switch((ADDR)addressing){
                    case REGIND: case REGDIR:
                    exec(instrAddr, opcode, regdescr, (ADDR)addressing, up); break;
                    case IMMED:
                    case REGPLUS:
                    case REGINDPLUS:
                    case MEM:
                    //cout << " with payload ";
                    datahigh = em->mem[pc++];
                    datalow = em->mem[pc++];
                    data = datalow + ((unsigned short)datahigh << 8);
                    exec(instrAddr, opcode, regdescr, (ADDR)addressing, up, data);
                    break;
                    default:
                    cout << "error at addr " << pc << ": undefined addressing mode\n";
                    exit(-102);
                }
            }
        }

        terminal->readInput();
        timer->tick();
        interrupts();
    }
    //cout << "End?" << endl;
    in.close();
}

void Interpreter::exec(int instrAddr, unsigned char opcode, unsigned char regdescr, 
    ADDR addressing,  unsigned char up, unsigned short data){
        short regS = regdescr & 0xf;
        short regD = (regdescr >> 4) & 0xf;
        //cout << "Executing instruction " << instrmap[opcode] << "\taddressing: " << addressing <<endl;
        switch(opcode){
            case 0: //HALT
            halt(); break;
            case 0x20: // IRET
            iret(); break;
            case 0x40:
            ret(); break;
            case 0x10: // INT
            interrupt(regD); break;
            case 0x80:
            Not(regD); break;
            case 0xA0:
            ldr(regD, regS, addressing, up, data); break;
            case 0xB0:
            str(regD, regS, addressing, up, data);break;
            case 0x60:
            xcgh(regD, regS); break;
            case 0x70:
            add(regD, regS); break;
            case 0x71:
            sub(regD, regS); break;
            case 0x72:
            mul(regD, regS); break;
            case 0x73:
            div(regD, regS); break;
            case 0x74:
            cmp(regD, regS); break;
            case 0x81:
            And(regD, regS); break;
            case 0x82:
            Or(regD, regS); break;
            case 0x83:
            Xor(regD, regS); break;
            case 0x84:
            test(regD, regS); break;
            case 0x90:
            shl(regD, regS); break;
            case 0x91:
            shr(regD, regS); break;
            case 0x30:
            push(rpc); jmp(regD, regS, addressing, up, data); break;
            case 0x50:
            jmp(regD, regS, addressing, up, data); break;
            case 0x51:
            if(bitZ())
                jmp(regD, regS, addressing, up, data); 
            break;
            case 0x52:
            if(!bitZ())
                jmp(regD, regS, addressing, up, data); 
            break;
            case 0x53:
            // !Z & (C || (!C & !O & !N))
            if(!bitZ() && (bitC() || (!bitC() && !bitO() && !bitN()))) 
                jmp(regD, regS, addressing, up, data);
            break;
            default: 
            cout << "error at addr " << instrAddr << ": undefined instruction\n";
            exit(-101);
        }
}



unsigned short Interpreter::readWSmall(unsigned short addr){
    unsigned short low = (unsigned short)em->mem[addr] << 8;
    unsigned short high = em->mem[addr+1];
    return high | low;
}
unsigned short Interpreter::readWBig(unsigned short addr){
    unsigned short high = (unsigned short)em->mem[addr] << 8;
    unsigned short low = em->mem[addr+1];
    return high | low;
}
unsigned short Interpreter::readW(unsigned short addr){
    return readWBig(addr);
}

void Interpreter::storeW(unsigned short addr, unsigned short val){
    storeWBig(addr, val);
}

void Interpreter::storeWBig(unsigned short addr, unsigned short val){
    em->mem[addr] = (unsigned char)((val & 0xff00) >> 8);
    em->mem[addr+1] = val & 0x00ff;
}

void Interpreter::halt(){
        end = true;
    }

void Interpreter::iret(){
    pop(rpsw);
    pop(rpc);
}

void Interpreter::ret(){
    pop(rpc);
}

void Interpreter::pop(short regnum){
        unsigned char high = em->mem[sp++];
        unsigned char low = em->mem[sp++];
        if(regnum == 8){
            em->psw = (high<<4) | low;
        }
        else{
            em->reg[regnum] = (high<<4) | low;
        }
    }

void Interpreter::push(short regnum){
    unsigned char high, low;
    if(regnum==8){
        high = (em->psw >> 8) & 0xff;
        low = em->psw & 0xff;
    }
    else{
        high = (em->reg[regnum] >> 8) & 0xff;
        low = em->reg[regnum] & 0xff;
    }
    em->mem[--sp] = low;
    em->mem[--sp] = high;
}

void Interpreter::interrupt(short regnum){
    int_i(em->reg[regnum]);
}

void Interpreter::int_i(int i){
    push(rpc);
    push(rpsw);
    unsigned short newpc;
    newpc = (((unsigned short)em->ivt[(i % em->nivt) * 2]) << 8)
    + (((unsigned short)em->ivt[(i % em->nivt) * 2 + 1]) & 0xff);
    //cout << "INT " << i << "- new pc=" << newpc << endl;
    em->reg[rpc] = newpc;
    // INTERRUPT MASKING
    em->psw_seti(1);
}
void Interpreter::interrupts(){
    
    if((em->psw & 0x8000) != 0) return;
    //cout << "PSW=" << (unsigned short)em->psw << "   " << "ireq=" << em->int_req << endl;
    unsigned short pswIntrMask[] = {0x0, 0x0, 0x4000, 0x2000};
    //cout << "Checking for interrupt requests...\n";
    for(int i=2;i<4;i++){
        if(((em->int_req & (1<<i)) != 0) && ((em->psw && pswIntrMask[i]) != 0)){
            //cout << "int_req_before=" << em->int_req << endl;
            em->int_req &= ~(1<<i);
            //cout << "int_req_after=" << em->int_req << endl;
            int_i(i);
            break;
        }
    }
}



void Interpreter::xcgh(short regD, short regS){
    short temp = em->reg[regS];
    em->reg[regS] = em->reg[regD];
    em->reg[regD] = temp;
}

void Interpreter::add(short regD, short regS){
    //cout << "ADD" << endl;
    em->reg[regD] += em->reg[regS];
}

void Interpreter::sub(short regD, short regS){
    em->reg[regD] -= em->reg[regS];
}

void Interpreter::mul(short regD, short regS){
    em->reg[regD] *= em->reg[regS];
}

void Interpreter::div(short regD, short regS){
    em->reg[regD] /= em->reg[regS];
}

void Interpreter::cmp(short regD, short regS){
    unsigned int tmp1 = (unsigned int)em->reg[regD];
    unsigned int tmp2 = (unsigned int)em->reg[regS];
    unsigned int tmpint = tmp1 - tmp2;
    bool carry = (tmpint >= 0x10000);
    short tmp = em->reg[regD] - em->reg[regS];

    if(tmp == 0) em->psw_setz(1);
    else em->psw_setz(0);

    if(tmp < 0) em->psw_setn(1);
    else em->psw_setn(0);

    if(carry) em->psw_setc(1);
    else em->psw_setc(0);

    if(((short)em->reg[regS] > 0 && tmp > (short)em->reg[regD]) ||
    ((short)em->reg[regS] < 0 && tmp < (short)em->reg[regD]))
        em->psw_seto(1);
    else
        em->psw_seto(0);
}

void Interpreter::Not(short regD){
    em->reg[regD] = ~em->reg[regD];
}

void Interpreter::And(short regD, short regS){
    em->reg[regD] = em->reg[regD] & em->reg[regS];
}

void Interpreter::Or(short regD, short regS){
    em->reg[regD] = em->reg[regD] | em->reg[regS];
}

void Interpreter::Xor(short regD, short regS){
    em->reg[regD] = em->reg[regD] ^ em->reg[regS];
}

void Interpreter::test(short regD, short regS){
    short temp = em->reg[regD] = em->reg[regD] & em->reg[regS];
    if(temp == 0) em->psw_setz(1);
    else em->psw_setz(0);
}


void Interpreter::shl(short regD, short regS){
    unsigned short prev = em->reg[regD];
    unsigned int intres = em->reg[regD] << em->reg[regS];
    em->reg[regD] <<= em->reg[regS];

    if(em->reg[regD] == 0) em->psw_setz(1);
    else em->psw_setz(0);

    if(em->reg[regD] < 0) em->psw_setn(1);
    else em->psw_setn(0);

    int cnt = 0;
    int mask = 1<<16;
    while(prev & mask != 0){
        cnt++;
        mask >>= 1;
    }
    if(em->reg[regS] > cnt) em->psw_seto(1);
    else em->psw_seto(0);
}

void Interpreter::shr(short regD, short regS){
    unsigned short prev = em->reg[regD];
    unsigned int intres = em->reg[regD] >> em->reg[regS];
    em->reg[regD] >>= em->reg[regS];

    if(em->reg[regD] == 0) em->psw_setz(1);
    else em->psw_setz(0);

    if(em->reg[regD] < 0) em->psw_setn(1);
    else em->psw_setn(0);

    int cnt = 0;
    int mask = 1;
    while(prev & mask != 0){
        cnt++;
        mask <<= 1;
    }
    if(em->reg[regS] > cnt) em->psw_seto(1);
    else em->psw_seto(0);
}


/*

IMMED = 0, REGDIR = 1, REGPLUS = 5, REGIND = 2, 
    REGINDPLUS = 3, MEM = 4

*/

void Interpreter::ldr(short regD, short regS, short addressing, unsigned char up, unsigned short data){
    unsigned short operand = fetchOperandLd(regS, addressing, up, data);
    if((ADDR)addressing == IMMED || (ADDR)addressing == REGDIR)
        em->reg[regD] = operand;
    else
        operand = readW(operand);
    
    em->reg[regD] = operand;

}

void Interpreter::str(short regD, short regS, short addressing, unsigned char up, unsigned short data){
    unsigned short operand;
    switch((ADDR)addressing){
        case IMMED:
            operand = data; break;
        case REGDIR:
            em->reg[regD] = em->reg[regS]; return;
        case REGPLUS:
            operand = em->reg[regS] + data; break;
        case REGIND:
            if(up == 1)
                em->reg[regS]-=2;
            if(up == 2)
                em->reg[regS]+=2;
            operand = em->reg[regS];
            if(up == 3)
                em->reg[regS]-=2;
            if(up == 4)
                em->reg[regS]+=2;
            break;
        case REGINDPLUS:
            operand = em->reg[regS]+data; break;
        case MEM:
            operand = data; break;
    }
        
    em->mem[operand] = (unsigned char)((em->reg[regD] & 0xff00) >> 8);
    em->mem[operand+1] =  (unsigned char)(em->reg[regD] & 0xff);

    if(operand == Terminal::out){
        //cout << "=================TERMINAL_OUT==================\n\n\n\n";
        cout << (char)em->mem[Terminal::out+1];//"|" << (char)em->mem[Terminal::out] << endl;
        //cout << "\n\n\n=================TERMINAL_OUT==================";
    }
}

void Interpreter::jmp(short regD, short regS, short addressing, unsigned char up, unsigned short data){
    short operand = fetchOperandLd(regS, addressing, up, data);
    if((ADDR)addressing == IMMED || (ADDR)addressing == REGDIR)
        pc = operand;
    else
        operand = em->mem[operand];
    pc = operand;
}
    

unsigned short Interpreter::fetchOperandLd(short regS, short addressing, unsigned char up, unsigned short data){
    unsigned short ret = -1;
    switch((ADDR)addressing){
        case IMMED:
            return data;
        case REGDIR:
            return em->reg[regS];
        case REGPLUS:
            // pcrel
            return em->reg[regS] + data;
        case REGIND:
            if(up == 1)
                em->reg[regS]-=2;
            if(up == 2)
                em->reg[regS]+=2;
            ret = em->reg[regS];
            if(up == 3)
                em->reg[regS]-=2;
            if(up == 4)
                em->reg[regS]+=2;
            break;
        case REGINDPLUS:
            return em->reg[regS]+data;
        case MEM:
            return data;
    }
    return ret;
}

