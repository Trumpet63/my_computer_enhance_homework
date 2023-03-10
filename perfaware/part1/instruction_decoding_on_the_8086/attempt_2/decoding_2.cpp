#include <stdio.h>
#include <string.h>

typedef unsigned char BYTE;

#define getOpcode(target) getBits(target, 0, 5);
#define getD(target)      getBits(target, 6, 6);
#define getW(target)      getBits(target, 7, 7);
#define getMod(target)    getBits(target, 8, 9);
#define getReg(target)    getBits(target, 10, 12);
#define getRm(target)     getBits(target, 13, 15);

// [num_rows][num_cols][max_string_length+1]
const char registerNameLookup[8][2][3] = {
    {"al", "ax"},
    {"cl", "cx"},
    {"dl", "dx"},
    {"bl", "bx"},
    {"ah", "sp"},
    {"ch", "bp"},
    {"dh", "si"},
    {"bh", "di"},
};

// Taken from https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
void printbits(unsigned short x) {
    for(int i=sizeof(x)<<3; i; i--)
        putchar('0'+((x>>(i-1))&1));
}

unsigned short getBits(unsigned short target, int start, int end) {
    unsigned short shifted = target >> (15 - end);
    int getLength = end - start + 1;
    unsigned short mask = (1 << getLength) - 1;
    return shifted & mask;
}

void processInstruction(char outBuffer[16], unsigned short instruction) {
    // printf("instruction: ");
    // printbits(instruction);

    // should always be 100010 for this exercise (the mov operation)
    unsigned short opcode = getOpcode(instruction);

    // 1 if reg indicates the destination, 0 otherwise (in which case rm is the destination)
    unsigned short d = getD(instruction);

    // 1 if the operation operates on 16 bits (wide), 0 otherwise (in which case it's 8 bits)
    unsigned short w = getW(instruction);

    // mod should be 11 for this exercise, which denotes a register-to-register operation
    unsigned short mod = getMod(instruction);

    unsigned short reg = getReg(instruction);
    unsigned short rm = getRm(instruction);

    // printf("\nopcode: ");
    // printbits(opcode);
    // printf("\nd: ");
    // printbits(d);
    // printf("\nw: ");
    // printbits(w);
    // printf("\nmod: ");
    // printbits(mod);
    // printf("\nreg: ");
    // printbits(reg);
    // printf("\nrm: ");
    // printbits(rm);

    const char* regName = registerNameLookup[reg][w];
    const char* rmName = registerNameLookup[rm][w];

    if (d) {
        snprintf(outBuffer, 16, "mov %s, %s\n", regName, rmName);
    } else {
        snprintf(outBuffer, 16, "mov %s, %s\n", rmName, regName);
    }
}

int main(void) {
    // const char* infile_path = "../../listing_0037_single_register_mov";
    const char* infile_path = "../../listing_0038_many_register_mov";
    // const char* outfile_path = "answer_0037.asm";
    const char* outfile_path = "answer_0038.asm";

    const int bufferSize = 1000;
    BYTE buffer[bufferSize];
    FILE* infile = fopen(infile_path, "rb");
    if (infile == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    // Note: I read the whole file in, as opposed to reading in an unsigned short
    // repeatedly because otherwise endianness gets messed up
    size_t bytes_read = fread(buffer, sizeof(BYTE), bufferSize, infile);
    fclose(infile);

    FILE* outfile = fopen(outfile_path, "w");
    fprintf(outfile, "bits 16\n");
    for (int i = 0; i < bytes_read; i+=2) {
        // Interpret two bytes as a single, two-byte number
        unsigned short instruction = buffer[i] << 8 | buffer[i+1];
        char outBuffer[16];
        processInstruction(outBuffer, instruction);
        fprintf(outfile, outBuffer);
    }
    fclose(outfile);
}
