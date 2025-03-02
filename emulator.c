#include <stdio.h>
#include <unistd.h>


unsigned char memory[1024 * 64] = {};

unsigned int stackPointer = 1024 * 32;
unsigned int programCounter = 0;

unsigned int carryFlag = 0;
unsigned int haltFlag = 0;

short a = 0;
short b = 0;



#define OPCODE_BITMASK 0b11111111000000000000000000000000
#define OPCODE_BITSHIFT 24

//instructions
void hlt(unsigned int instruction) {
    haltFlag = 1;
    programCounter += 4;
}

void add(unsigned int instruction) {
    carryFlag = ((int)a + (int)b) > 0x7FFF;
    a = a + b;
    programCounter += 4;
}
void sub(unsigned int instruction) {
    carryFlag = ((int)a - (int)b) < -0x8000;
    a = a - b;
    programCounter += 4;
}
void mul(unsigned int instruction) {
    a = a * b;
    programCounter += 4;
}
void div(unsigned int instruction) {
    a = a / b;
    programCounter += 4;
}


void ltb(unsigned int instruction) {
    a = a < b;
    programCounter += 4;
}
void bsr(unsigned int instruction) {
    a = a >> b;
    programCounter += 4;
}
void bsl(unsigned int instruction) {
    a = a << b;
    programCounter += 4;
}
void mod(unsigned int instruction) {
    a = a % b;
    programCounter += 4;
}


void bio(unsigned int instruction) {
    a = a | b;
    programCounter += 4;
}
void bia(unsigned int instruction) {
    a = a & b;
    programCounter += 4;
}
void bix(unsigned int instruction) {
    a = a ^ b;
    programCounter += 4;
}
void bin(unsigned int instruction) {
    a = ~a;
    programCounter += 4;
}

void blo(unsigned int instruction) {
    a = (a != 0) || (b != 0);
    programCounter += 4;
}
void bla(unsigned int instruction) {
    a = (a != 0) && (b != 0);
    programCounter += 4;
}
void blx(unsigned int instruction) {
    a = (a != 0) ^ (b != 0);
    programCounter += 4;
}
void bln(unsigned int instruction) {
    a = a != 0;
    programCounter += 4;
}

void teq(unsigned int instruction) {
    a = a == b;
    programCounter += 4;
}


void jmp(unsigned int instruction) {
    programCounter = b;
}
void jez(unsigned int instruction) {
    if (a == 0) {
        programCounter = b;
    }
    else {
        programCounter += 4;
    }
}
void jnz(unsigned int instruction) {
    if (a != 0) {
        programCounter = b;
    }
    else {
        programCounter += 4;
    }
}


void lia(unsigned int instruction) {
    a = instruction & 0x0000FFFF;
    programCounter += 4;
}
void lib(unsigned int instruction) {
    b = instruction & 0x0000FFFF;
    programCounter += 4;
}
void swp(unsigned int instruction) {
    int c = a;

    a = b;
    b = c;

    programCounter += 4;
}

void lma(unsigned int instruction) {
    a = (memory[b] << 8) + memory[b + 1];
    programCounter += 4;
}
void sma(unsigned int instruction) {
    memory[b + 0] = (a & 0xFF00) >> 8;
    memory[b + 1] =  a & 0x00FF;
    programCounter += 4;
}
void lra(unsigned int registerIdentifier) {
    programCounter += 4;
    switch(registerIdentifier) {
        case 0:
            a = stackPointer;
            return;
        case 1:
            a = programCounter;
            return;
        case 2:
            a = carryFlag;
            return;
    }

    printf("Invalid lra identifier: %d", registerIdentifier);
    hlt(0);
}

void pop(unsigned int instruction) {
    a = (memory[stackPointer] << 8) + memory[stackPointer + 1];
    stackPointer -= 2;
    programCounter += 4;
}
void psh(unsigned int instruction) {
    stackPointer += 2;

    memory[stackPointer + 0] = (a & 0xFF00) >> 8;
    memory[stackPointer + 1] =  a & 0x00FF;
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
    printf("a=%i b=%i sp=%i pc=%i cf=%i hf=%i\n", a, b, stackPointer, programCounter, carryFlag, haltFlag);
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
    }
}