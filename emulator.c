#include <stdio.h>
#include <unistd.h>


unsigned char memory[1024 * 64] = {};

unsigned int stackPointer = 1024 * 32;
unsigned int programCounter = 0;

unsigned int carryFlag = 0;
unsigned int haltFlag = 0;


typedef union {
    short asSigned;
    unsigned short asUnsigned;
} twoTypeRegister;
twoTypeRegister a = { .asUnsigned = 0 };
twoTypeRegister b = { .asUnsigned = 0 };



#define OPCODE_BITMASK 0b11111111000000000000000000000000
#define OPCODE_BITSHIFT 24

#define LRA_SP 0
#define LRA_PC 1
#define LRA_CF 2


//instructions
void hlt(unsigned int instruction) {
    haltFlag = 1;
    programCounter += 4;
}

void add(unsigned int instruction) {
    carryFlag = (a.asUnsigned + b.asUnsigned) > 0x7FFF;
    a.asSigned = a.asSigned + b.asSigned;
    programCounter += 4;
}
void sub(unsigned int instruction) {
    carryFlag = (a.asUnsigned - b.asUnsigned) < -0x8000;
    a.asSigned = a.asSigned - b.asSigned;
    programCounter += 4;
}
void mul(unsigned int instruction) {
    a.asSigned = a.asSigned * b.asSigned;
    programCounter += 4;
}
void div(unsigned int instruction) {
    a.asSigned = a.asSigned / b.asSigned;
    programCounter += 4;
}


void ltb(unsigned int instruction) {
    a.asSigned = a.asSigned < b.asSigned;
    programCounter += 4;
}
void bsr(unsigned int instruction) {
    a.asSigned = a.asSigned >> b.asSigned;
    programCounter += 4;
}
void bsl(unsigned int instruction) {
    a.asSigned = a.asSigned << b.asSigned;
    programCounter += 4;
}
void mod(unsigned int instruction) {
    a.asSigned = a.asSigned % b.asSigned;
    programCounter += 4;
}


void bio(unsigned int instruction) {
    a.asUnsigned = a.asUnsigned | b.asUnsigned;
    programCounter += 4;
}
void bia(unsigned int instruction) {
    a.asUnsigned = a.asUnsigned & b.asUnsigned;
    programCounter += 4;
}
void bix(unsigned int instruction) {
    a.asUnsigned = a.asUnsigned ^ b.asUnsigned;
    programCounter += 4;
}
void bin(unsigned int instruction) {
    a.asUnsigned = ~a.asUnsigned;
    programCounter += 4;
}

void blo(unsigned int instruction) {
    a.asUnsigned = (a.asUnsigned != 0) || (b.asUnsigned != 0);
    programCounter += 4;
}
void bla(unsigned int instruction) {
    a.asUnsigned = (a.asUnsigned != 0) && (b.asUnsigned != 0);
    programCounter += 4;
}
void blx(unsigned int instruction) {
    a.asUnsigned = (a.asUnsigned != 0) ^ (b.asUnsigned != 0);
    programCounter += 4;
}
void bln(unsigned int instruction) {
    a.asUnsigned = a.asUnsigned != 0;
    programCounter += 4;
}

void teq(unsigned int instruction) {
    a.asUnsigned = a.asUnsigned == b.asUnsigned;
    programCounter += 4;
}


void jmp(unsigned int instruction) {
    programCounter = b.asUnsigned;
}
void jez(unsigned int instruction) {
    if (a.asUnsigned == 0) {
        programCounter = b.asUnsigned;
    }
    else {
        programCounter += 4;
    }
}
void jnz(unsigned int instruction) {
    if (a.asUnsigned != 0) {
        programCounter = b.asUnsigned;
    }
    else {
        programCounter += 4;
    }
}


void lia(unsigned int instruction) {
    a.asUnsigned = instruction & 0x0000FFFF;
    programCounter += 4;
}
void lib(unsigned int instruction) {
    b.asUnsigned = instruction & 0x0000FFFF;
    programCounter += 4;
}
void swp(unsigned int instruction) {
    twoTypeRegister c = a;

    a = b;
    b = c;

    programCounter += 4;
}

void lma(unsigned int instruction) {
    a.asUnsigned = (memory[b.asUnsigned] << 8) + memory[b.asUnsigned + 1];
    programCounter += 4;
}
void sma(unsigned int instruction) {
    memory[b.asUnsigned + 0] = (a.asUnsigned & 0xFF00) >> 8;
    memory[b.asUnsigned + 1] =  a.asUnsigned & 0x00FF;
    programCounter += 4;
}
void lra(unsigned int instruction) {
    programCounter += 4;
    switch(instruction & 0b11) {
        case LRA_SP:
            a.asUnsigned = stackPointer;
            return;
        case LRA_PC:
            a.asUnsigned = programCounter;
            return;
        case LRA_CF:
            a.asUnsigned = carryFlag;
            return;
    }

    printf("Invalid lra identifier: %d", instruction & 0b11);
    hlt(0);
}

void pop(unsigned int instruction) {
    a.asUnsigned = (memory[stackPointer - 2] << 8) + memory[stackPointer - 1];
    stackPointer -= 2;
    programCounter += 4;
}
void psh(unsigned int instruction) {
    memory[stackPointer + 0] = (a.asUnsigned & 0xFF00) >> 8;
    memory[stackPointer + 1] =  a.asUnsigned & 0x00FF;
    
    stackPointer += 2;
    programCounter += 4;
}

const void (*operations[32])(unsigned int instruction) = {
    hlt, 0, 0, 0,

    add, sub, mul, div,
    ltb, bsr, bsl, mod,

    bio, bia, bix, bin,
    blo, bla, blx, bln,

    teq, jmp, jez, jnz,

    lia, lib, swp, lma,
    sma, lra, pop, psh
}; 
const char* opnames[32] = {
    "hlt", "", "", "",

    "add", "sub", "mul", "div",
    "ltb", "bsr", "bsl", "mod",

    "bio", "bia", "bix", "bin",
    "blo", "bla", "blx", "bln",

    "teq", "jmp", "jez", "jnz",

    "lia", "lib", "swp", "lma",
    "sma", "lra", "pop", "psh",
};


void dump_registers() {
    printf("a=%i b=%i sp=%i pc=%i cf=%i hf=%i\n", a.asUnsigned, b.asUnsigned, stackPointer, programCounter, carryFlag, haltFlag);
}


int main(int argc, char* argv[]) {
    if(argc != 2){
        printf("Emulator was not provided with program binary. %d", argc);
        return -1;
    }

    FILE *fptr = fopen(argv[1], "r");
    if(fptr == NULL) {
        printf("Unable to open binary");
    }
    fgets(memory, (1024 * 32) - 1, fptr);


    while(haltFlag == 0) {
        unsigned int instruction = (memory[programCounter + 3])
                        + (memory[programCounter + 2] << 8)
                        + (memory[programCounter + 1] << 16)
                        + (memory[programCounter + 0] << 24);

        unsigned int opcode = (instruction & OPCODE_BITMASK) >> OPCODE_BITSHIFT;
        const char* opname = opnames[opcode];
        
        printf("executing %s: 0x%08X. postexec registers: ", opname, instruction);
        operations[opcode](instruction);

        dump_registers();
        usleep(500*1000);
    }
}
