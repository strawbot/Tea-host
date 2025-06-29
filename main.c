// Tea testing
#include "tea.c"

static Byte bit_sync[6] = {0xEB, 0x90, 0xB4, 0x33, 0xAA, 0xAA};
static Byte bit_seq[1000];

static Byte test_frame[] = {
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, // preamble
    0xEB, 0x90, 0xB4, 0x33, 0xAA, 0xAA, // bit sync
    0x35, 0x2E, 0xF8, 0x53, // frame sync
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 250, 251, 252, 253, 254, 255,
};

static Byte test_frame_bits[sizeof(test_frame) * 8];

static bool bit(Byte * p, Short bit) { return 1 & p[bit / 8] >> (7 - bit % 8); }

static void frame_bits(Byte * frame, Short n, Byte * bit_seq) {
    bool first = bit(frame, 0);
    Byte seq = 0;
    Byte * bp = bit_seq;
    for (Short i = 0; i < n * 8; i++) {
        bool b = bit(frame, i);
        if (b == first)
            seq++;
        else {
            *bp++ = seq;
            seq = 1;
            first = b;
        }
    }
    *bp++ = seq;
}

static void test_bits_sync() {
    frame_bits(bit_sync, sizeof(bit_sync), bit_seq);
    printCr();
    Byte * bp = bit_seq;
    while (*bp)
        printDec0(*bp++);

    frame_bits(test_frame, sizeof(test_frame), test_frame_bits);
    printCr();
    bp = test_frame_bits;
    while (*bp)
        printDec0(*bp++);
    }

static void init_app() {
    later(test_bits_sync);
}

int main() {
    init_tea();
    init_app();
    serve_tea();
    return 0;
}
