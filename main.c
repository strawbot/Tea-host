// Tea testing
#include "tea.c"

bool bit(Byte * p, Byte bit) { return 1 & p[bit / 8] >> (7 - bit % 8); }

void spit_bits_sync() {
    Byte bit_sync[6] = {0xEB, 0x90, 0xB4, 0x33, 0xAA, 0xAA};
    print("\nBit Sync in bits: ");
    bool first = bit(bit_sync, 0);
    Byte seq = 0;
    Byte bit_seq[1000] = {0};
    Byte * bp = bit_seq;
    for (Byte i = 0; i < sizeof(bit_sync) * 8; i++) {
        bool b = bit(bit_sync, i);
        printDec0(b);
        if (b == first)
            seq++;
        else {
            *bp++ = seq;
            seq = 1;
            first = b;
        }
    }
    *bp++ = seq;
    printCr();
    bp = bit_seq;
    while (*bp)
        printDec0(*bp++);
}

void init_app() {
    later(spit_bits_sync);
}

int main() {
    init_tea();
    init_app();
    serve_tea();
    return 0;
}
