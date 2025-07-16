#include "printers.h"
#include "tea.h"
#include <limits.h>
#include <string.h>

// typedef Short distance_t;
// static const distance_t distance_max = UINT16_MAX;

#define BSYNC_SIZE 6
#define FSYNC_SIZE 4
#define BIT_SYNC 0xEB, 0x90, 0xB4, 0x33, 0xAA, 0xAA
#define FS_MODE0 0x35, 0x2E, 0xF8, 0x53
#define FS_MODE1 0x58, 0x98, 0x23, 0x3E
#define FS_MODE2 0xEE, 0x43, 0x4E, 0x88

// framing
Byte *payload;

static Byte f_sync[][FSYNC_SIZE] = {{FS_MODE0}, {FS_MODE1}, {FS_MODE2}};
static Byte bit_sync[BSYNC_SIZE] = {BIT_SYNC};

static Byte bit_sync_bits[1000];

static Byte test_frame[] = {
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, // preamble
    0xEB, 0x90, 0xB4, 0x33, 0xAA, 0xAA, 0x35, 0x2E, 0xF8, 0x53, 0x0D, 0xC5,
    0xD4, 0x2F, 0xAE, 0x54, 0x25, 0x9D, 0x9E, 0x93, 0xF6, 0x6F, 0x78, 0xE6,
    0x16, 0xCD, 0xBA, 0x69, 0xAC, 0x67, 0x78, 0xA3, 0xAB, 0xC5, 0xB4, 0x73,
    0x1F, 0x90, 0x02, 0x6A, 0x7D, 0xB9, 0xC4, 0xB3, 0x00, 0xA3, 0x31, 0x6E,
    0x85, 0xBD, 0x0B, 0x45, 0x84, 0x4D, 0x44, 0xF1, 0x94, 0x71, 0x4C, 0x9C,
    0x97, 0x57, 0x71, 0xD8, 0x5F, 0x86, 0x47, 0x6B, 0x40, 0x18, 0xC5, 0x14,
    0xEC, 0x28, 0xC2, 0x07, 0xE9, 0xF1, 0x1C, 0x16, 0x7C, 0xAC, 0x93, 0x3B,
    0x69, 0x10, 0xB9, 0x4C, 0xE0, 0x8A, 0x39, 0xC0,
};

Byte test_frame_bits[5000];

void clear_test_frame() { memset(test_frame_bits, 0, sizeof(test_frame_bits)); }

// on STM32L4R5 use bit banding to read/write bits directly: SRAM1
static bool get_bit(Byte *p, Short bit) {
    return 1 & (p[bit / 8] >> (7 - bit % 8));
}
static void set_bit(Byte *p, Short bit) { p[bit / 8] |= 1 << (7 - bit % 8); }
static void set_bits(Byte *p, Short index, Short n) {
    while (n--)
        set_bit(p, index++);
}

static void
frame_bits(Byte *frame, Short n,
           Byte *bit_sync_bits) { // turn hex byte frame into bit sequences
    bool first = get_bit(frame, 0);
    Byte seq = 0;
    for (Short i = 0; i < n * 8; i++) {
        bool b = get_bit(frame, i);
        if (b == first)
            seq++;
        else {
            *bit_sync_bits++ = seq;
            seq = 1;
            first = b;
        }
    }
    *bit_sync_bits++ = seq;
}

static void build_bit_seqs() {
    frame_bits(bit_sync, sizeof(bit_sync), bit_sync_bits);
    frame_bits(test_frame, sizeof(test_frame), test_frame_bits);
}

// last bit might be 1 or more so it will match if equal to or >
// in the case the last bit is 1, it will always match except for end of string
static Short match_bit_seqs(Byte *sync, Byte *frame) {
    Short longest = 0;
    Byte *mp = sync;                          // match pointer
    Byte *sp = frame;                         // search pointer
    while (*sp && *mp)                        // end of either
        if (*sp++ != *mp++) {                 // when they differ
            if (*mp == 0 && mp[-1] <= sp[-1]) // check last match
                break;
            sp = sp - (mp - sync) + 1; // back to start of search plus 1
            longest = max(longest, mp - sync);
            mp = sync;
        }
    print("\nLongest match:"), printDec(longest);
    if (*mp == 0) { // check if match pointer complete
        sp -= strlen((char *)sync);
        return sp - frame;
    }
    return strlen((char *)test_frame_bits); // return end of frame for no match
}

static void get_frame_sync(Byte *frame, Short start) {
    Byte *fp = frame + start;
    Short n = 0;
    while (*fp)
        n += *fp++; // count bits

    if (n < FSYNC_SIZE * 8) {
        print("\nNot enuf bits.");
        return;
    }

    Byte pdu[n / 8 + 1]; // add extra byte for any over bits
    Short index = 0;

    memset(pdu, 0, sizeof(pdu));
    fp = frame + start;
    for (;;) {
        if ((n = *fp++)) {
            set_bits(pdu, index, n); // write one's
            index += n;
            if ((index += *fp++) == 0) // skip zero's
                break;
        } else
            break;
    }

    Byte *fec = pdu + BSYNC_SIZE;
    if (memcmp(f_sync[0], fec, FSYNC_SIZE) == 0)
        print(" FEC Mode 0 ");
    else if (memcmp(f_sync[1], fec, FSYNC_SIZE) == 0)
        print(" FEC Mode 1 ");
    else if (memcmp(f_sync[2], fec, FSYNC_SIZE) == 0)
        print(" FEC Mode 2 ");
    else
        print(" Unknown FEC mode or bit error ");
    print("\nPDU: "), hbytes(pdu, sizeof(pdu));
}

void print_results() {
    Short index = match_bit_seqs(bit_sync_bits, test_frame_bits);
    Short tlen = strlen((char *)test_frame_bits);
    if (index == tlen)
        print("\nNo match");
    else {
        print("\nBit match at: "), printDec(index);
        print(" out of: "), printDec(tlen);
        get_frame_sync(test_frame_bits, index);
    }
}

/*
1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 2 1 2 1 3 2 1
3 3 1 1 1 1 1 1 1 3 1 1 2 2 3 2 1 2 3 2 2 2 1 3 2 2 1 1 2 2 2 1 3 1 1 2 1 2 4 1
1 2 4 1 4 1 3 1 4 8 1 2 6 2 1 1 1 3 6 3 3 1 2 3 1 3 4 1 3 1 2 2 1 4 2 1 1 2 4 1
2 1 4 4 2 1 3 3 2 2 1 2 2 2 1 3 1 2 5 1 1 2 2 2 1 2 1 2 2 1 4 2 2 1 3 4 1 2 1 2
1 1 1 3 1 3 3 2 2 2 1 2 1 1 1 2 1 2 1 3 2 3 2 2 0 1 2 2 2 2 1 2 4 1 3 1 1 3 3 3
3 2 5 1 2 3 1 2 4 1 2 2 2 1 3 2 2 1 2 1
*/
// test
void init_zc() {
    later(build_bit_seqs);
    later(print_results);
}
