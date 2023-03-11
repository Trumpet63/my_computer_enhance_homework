#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef unsigned char BYTE;

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

const char effectiveAddressExpressionLookup[8][8] = {
    "bx + si",
    "bx + di",
    "bp + si",
    "bp + di",
    "si",
    "di",
    "bp",
    "bx",
}; 

const size_t outBufferSize = 32;

// Taken from https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
void printbits(uint16_t x) {
    for(int i=sizeof(x)<<3; i; i--)
        putchar('0'+((x>>(i-1))&1));
}

void printbits8(uint8_t x) {
    for(int i=sizeof(x)<<3; i; i--)
        putchar('0'+((x>>(i-1))&1));
}

uint16_t getBits16(uint16_t target, int start, int end) {
    uint16_t shifted = target >> (15 - end);
    int getLength = end - start + 1;
    uint16_t mask = (1 << getLength) - 1;
    return shifted & mask;
}

uint8_t getBits8(uint8_t target, int start, int end) {
    uint8_t shifted = target >> (7 - end);
    int getLength = end - start + 1;
    uint8_t mask = (1 << getLength) - 1;
    return shifted & mask;
}

void decodeMovNoDisp(char outBuffer[outBufferSize], BYTE* bufferPointer, uint8_t rm) {
    uint8_t d = getBits8(bufferPointer[0], 6, 6);
    uint8_t w = getBits8(bufferPointer[0], 7, 7);
    uint8_t reg = getBits8(bufferPointer[1], 2, 4);

    const char* regName = registerNameLookup[reg][w];
    const char* expression = effectiveAddressExpressionLookup[rm];

    if (d) {
        snprintf(outBuffer, outBufferSize, "mov %s, [%s]\n", regName, expression);
    } else {
        snprintf(outBuffer, outBufferSize, "mov [%s], %s\n", expression, regName);
    }
}

void decodeMovRawDisp(char outBuffer[outBufferSize], BYTE* bufferPointer) {
    uint8_t d = getBits8(bufferPointer[0], 6, 6);
    uint8_t w = getBits8(bufferPointer[0], 7, 7);
    uint8_t reg = getBits8(bufferPointer[1], 2, 4);
    uint8_t disp_lo = bufferPointer[2];
    uint8_t disp_high = bufferPointer[3];

    const char* regName = registerNameLookup[reg][w];
    uint16_t disp = disp_high << 8 | disp_lo;

    if (d) {
        snprintf(outBuffer, outBufferSize, "mov %s, [%d]\n", regName, disp);
    } else {
        snprintf(outBuffer, outBufferSize, "mov [%d], %s\n", disp, regName);
    }
}

void decodeMov8(char outBuffer[outBufferSize], BYTE* bufferPointer) {
    uint8_t d = getBits8(bufferPointer[0], 6, 6);
    uint8_t w = getBits8(bufferPointer[0], 7, 7);
    uint8_t reg = getBits8(bufferPointer[1], 2, 4);
    uint8_t rm = getBits8(bufferPointer[1], 5, 7);

    const char* regName = registerNameLookup[reg][w];
    const char* expression = effectiveAddressExpressionLookup[rm];
    uint8_t disp_lo = bufferPointer[2];

    if (d) {
        snprintf(outBuffer, outBufferSize, "mov %s, [%s + %d]\n", regName, expression, disp_lo);
    } else {
        snprintf(outBuffer, outBufferSize, "mov [%s + %d], %s\n", expression, disp_lo, regName);
    }
}

void decodeMov16(char outBuffer[outBufferSize], BYTE* bufferPointer) {
    uint8_t d = getBits8(bufferPointer[0], 6, 6);
    uint8_t w = getBits8(bufferPointer[0], 7, 7);
    uint8_t reg = getBits8(bufferPointer[1], 2, 4);
    uint8_t rm = getBits8(bufferPointer[1], 5, 7);
    uint8_t disp_lo = bufferPointer[2];
    uint8_t disp_high = bufferPointer[3];

    const char* regName = registerNameLookup[reg][w];
    const char* expression = effectiveAddressExpressionLookup[rm];
    uint16_t disp = disp_high << 8 | disp_lo;

    if (d) {
        snprintf(outBuffer, outBufferSize, "mov %s, [%s + %d]\n", regName, expression, disp);
    } else {
        snprintf(outBuffer, outBufferSize, "mov [%s + %d], %s\n", expression, disp, regName);
    }
}

void decodeMovRegToReg(char outBuffer[outBufferSize], BYTE* bufferPointer) {
    uint8_t d = getBits8(bufferPointer[0], 6, 6);
    uint8_t w = getBits8(bufferPointer[0], 7, 7);
    uint8_t reg = getBits8(bufferPointer[1], 2, 4);
    uint8_t rm = getBits8(bufferPointer[1], 5, 7);

    const char* regName = registerNameLookup[reg][w];
    const char* rmName = registerNameLookup[rm][w];

    if (d) {
        snprintf(outBuffer, outBufferSize, "mov %s, %s\n", regName, rmName);
    } else {
        snprintf(outBuffer, outBufferSize, "mov %s, %s\n", rmName, regName);
    }
}

void decodeMovImmToReg8(char outBuffer[outBufferSize], BYTE* bufferPointer, uint8_t w) {
    uint8_t reg = getBits8(bufferPointer[0], 5, 7);

    const char* regName = registerNameLookup[reg][w];
    uint8_t immediate_lo = bufferPointer[1];

    snprintf(outBuffer, outBufferSize, "mov %s, %d\n", regName, immediate_lo);
}

void decodeMovImmToReg16(char outBuffer[outBufferSize], BYTE* bufferPointer, uint8_t w) {
    uint8_t reg = getBits8(bufferPointer[0], 5, 7);
    uint8_t immediate_lo = bufferPointer[1];
    uint8_t immediate_high = bufferPointer[2];
    
    const char* regName = registerNameLookup[reg][w];
    uint16_t immediate = immediate_high << 8 | immediate_lo;

    snprintf(outBuffer, outBufferSize, "mov %s, %d\n", regName, immediate);
}

int main(void) {
    // const char* infile_path = "../../listing_0037_single_register_mov";
    // const char* infile_path = "../../listing_0038_many_register_mov";
    const char* infile_path = "../../listing_0039_more_movs";
    // const char* outfile_path = "answer_0037.asm";
    // const char* outfile_path = "answer_0038.asm";
    const char* outfile_path = "answer_0039.asm";

    // Read the file at infile_path to the variable "buffer"
    FILE* infile = fopen(infile_path, "rb");
    if (infile == NULL) {
        printf("Error opening file\n");
        return 1;
    }
    const int bufferSize = 1000;
    BYTE buffer[bufferSize];
    size_t file_length = fread(buffer, sizeof(BYTE), bufferSize, infile);
    fclose(infile);

    BYTE* buffer_ptr = buffer;
    BYTE* buffer_end = buffer_ptr + file_length;
    FILE* outfile = fopen(outfile_path, "w");
    fprintf(outfile, "bits 16\n");
    // uint8_t debug_counter = 0;
    while (buffer_ptr < buffer_end) {
        // if (debug_counter > 4) {
        //     break;
        // }
        // printbits8(buffer_ptr[0]);
        // printf(" ");
        // printbits8(buffer_ptr[1]);
        // printf(" ");
        // printbits8(buffer_ptr[2]);
        // printf(" ");
        // printbits8(buffer_ptr[3]);
        // printf("\n");

        char outBuffer[outBufferSize];

        uint8_t shortOpcode = getBits8(buffer_ptr[0], 0, 3);
        if (shortOpcode == 0b1011) {
            uint8_t w = getBits8(buffer_ptr[0], 4, 4);
            if (w) {
                decodeMovImmToReg16(outBuffer, buffer_ptr, w);
                buffer_ptr += 3;
            } else {
                decodeMovImmToReg8(outBuffer, buffer_ptr, w);
                buffer_ptr += 2;
            }
        } else {
            uint8_t opcode = getBits8(buffer_ptr[0], 0, 5);
            uint8_t mod = getBits8(buffer_ptr[1], 0, 1);
            if (opcode == 0b00100010) {
                if (mod == 0) {
                    uint8_t rm = getBits8(buffer_ptr[1], 5, 7);
                    if (rm == 0b110) {
                        decodeMovRawDisp(outBuffer, buffer_ptr);
                        buffer_ptr += 4;
                    } else {
                        decodeMovNoDisp(outBuffer, buffer_ptr, rm);
                        buffer_ptr += 2;
                    }
                } else if (mod == 1) {
                    decodeMov8(outBuffer, buffer_ptr);
                    buffer_ptr += 3;
                } else if (mod == 2) {
                    decodeMov16(outBuffer, buffer_ptr);
                    buffer_ptr += 4;
                } else if (mod == 3) {
                    decodeMovRegToReg(outBuffer, buffer_ptr);
                    buffer_ptr += 2;
                }
            }
        }

        // printf(outBuffer);
        // debug_counter++;
        fprintf(outfile, outBuffer);
    }
    fclose(outfile);
}
