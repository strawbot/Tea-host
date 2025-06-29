// Tea testing
#include "tea.c"
#include <string.h>

static Byte bit_sync[6] = {0xEB, 0x90, 0xB4, 0x33, 0xAA, 0xAA};
static Byte bit_sync_bits[1000];

static Byte test_frame[] = {
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, // preamble
    0xEB, 0x90, 0xB4, 0x33, 0xAA, 0xAA, // bit sync
    0x35, 0x2E, 0xF8, 0x53, // frame sync
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 250, 251, 252, 253, 254, 255,
};

static Byte test_frame_bits[sizeof(test_frame) * 8];

static bool bit(Byte * p, Short bit) { return 1 & p[bit / 8] >> (7 - bit % 8); }

static void frame_bits(Byte * frame, Short n, Byte * bit_sync_bits) {
    bool first = bit(frame, 0);
    Byte seq = 0;
    Byte * bp = bit_sync_bits;
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

static void build_bit_seqs() {
    frame_bits(bit_sync, sizeof(bit_sync), bit_sync_bits);
    frame_bits(test_frame, sizeof(test_frame), test_frame_bits);
}

// last bit might be 1 or more so it will match if equal to or > 
// in the case the last bit is 1, it will always match except for end of string
static Short match_bit_seqs(Byte * sync, Byte * frame) {
    Byte * mp = sync;
    Byte * sp = frame;
    while (*sp && *mp)
        if (*sp++ != *mp++) {
            if (*mp == 0 && mp[-1] <= sp[-1]) // check last match
                break;
            sp -= mp - sync - 1; // back to start of search plus 1
            mp = sync;
        }
    if (*mp == 0) {
        sp -= strlen((char *)sync);
        return sp - frame;
    }
    return strlen((char*)test_frame_bits);
}

static void print_results() {
    Short index = match_bit_seqs(bit_sync_bits, test_frame_bits);
    Short tlen = strlen((char*)test_frame_bits);
    if (index == tlen)
        print("\nNo match");
    else {
        print("\nBit match at: "), printDec(index);
        print(" out of: "), printDec(tlen);
    }
}

static void init_app() {
    later(build_bit_seqs);
    later(print_results);
}

int main() {
    init_tea();
    init_app();
    serve_tea();
    return 0;
}
