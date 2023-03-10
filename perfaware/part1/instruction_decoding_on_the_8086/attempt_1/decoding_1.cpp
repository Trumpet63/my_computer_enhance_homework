#include <stdio.h>

// Taken from https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
void printbits(int x)
{
    for(int i=sizeof(x)<<3; i; i--)
        putchar('0'+((x>>(i-1))&1));
}

int main(void) {
    int some_int = (1 << 4) - 1;
    printf("Four ones: ");
    printbits(some_int);
}
