#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "printers.h"
#include "encode.h"

void fail_test() {
    printf("FAILED\n");
    exit(1);
}

void pass_test() { printf("PASSED\n"); }

// basic go no go test
#define DATA_SIZE 30
#define PARITY_SIZE 20

static Byte data[DATA_SIZE+PARITY_SIZE], *parity = data+DATA_SIZE;

int main() {
    memmove(data, "123456789112345678921234567893", DATA_SIZE);
    memset(parity, 0, PARITY_SIZE);
    encode_rs012(data, parity, DATA_SIZE);
    print("\nData:   "), hbytes(data, 30);
    print("\nParity: "), hbytes(parity, 20);
    printCr();
    return 0;
}
