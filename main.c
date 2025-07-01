// Tea testing
#include "tea.c"
#include <string.h>

#define BSYNC_SIZE 6
#define FSYNC_SIZE 4
#define BIT_SYNC 0xEB, 0x90, 0xB4, 0x33, 0xAA, 0xAA
#define FS_MODE0 0x35, 0x2E, 0xF8, 0x53
#define FS_MODE1 0x58, 0x98, 0x23, 0x3E
#define FS_MODE2 0xEE, 0x43, 0x4E, 0x88

static Byte f_sync[][FSYNC_SIZE] = {{FS_MODE0}, {FS_MODE1}, {FS_MODE2}};
static Byte bit_sync[BSYNC_SIZE] = {BIT_SYNC};

static Byte bit_sync_bits[1000];

static Byte test_frame[] = {
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, // preamble
    0xEB, 0x90, 0xB4, 0x33, 0xAA, 0xAA, // bit sync
    0x35, 0x2E, 0xF8, 0x53, // frame sync
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 250, 251, 252, 253, 254, 255,
};

static Byte test_frame_bits[sizeof(test_frame) * 8];

static bool get_bit(Byte * p, Short bit) { return 1 & p[bit / 8] >> (7 - bit % 8); }
static void set_bit(Byte * p, Short bit) { p[bit / 8] |= 1 << (7 - bit % 8); }

static void frame_bits(Byte * frame, Short n, Byte * bit_sync_bits) {
    bool first = get_bit(frame, 0);
    Byte seq = 0;
    Byte * bp = bit_sync_bits;
    for (Short i = 0; i < n * 8; i++) {
        bool b = get_bit(frame, i);
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

static void get_frame_sync(Byte * frame, Short start) {
    Byte * fp = frame + start;
    Short n = 0;
    while (*fp)
        n += *fp++;

    Byte pdu[n/8 + 1];
    memset(pdu, 0, sizeof(pdu));
    fp = frame + start;
    Short index = 0;
    bool one = true;
    while (*fp) {
        Byte n = *fp++;
        if (one)
            while (n--)
                set_bit(pdu, index++);
        else
            index += n;
        one = !one;
    }
    if (memcmp(f_sync[0], pdu + BSYNC_SIZE, FSYNC_SIZE) == 0)
        print(" FEC Mode 0 ");
    else if (memcmp(f_sync[1], pdu + BSYNC_SIZE, FSYNC_SIZE) == 0)
        print(" FEC Mode 1 ");
    else if (memcmp(f_sync[2], pdu + BSYNC_SIZE, FSYNC_SIZE) == 0)
        print(" FEC Mode 2 ");
    else
        print(" Unknown FEC mode or bit error ");
    print("\nPDU: "), hbytes(pdu, sizeof(pdu));
}

static void print_results() {
    Short index = match_bit_seqs(bit_sync_bits, test_frame_bits);
    Short tlen = strlen((char*)test_frame_bits);
    if (index == tlen)
        print("\nNo match");
    else {
        print("\nBit match at: "), printDec(index);
        print(" out of: "), printDec(tlen);
        get_frame_sync(test_frame_bits, index);
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
