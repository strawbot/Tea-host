/*
transmit for FEC mode 0 17 pdu bytes in 235 ms
alpdu:  00 00 10 0B 0B B8 34 5A E3 01 06 08 11 84 C9 11 04
encoded:  EB 90 B4 33 AA AA 35 2E F8 53 0D C5 D4 2F AE 54 25 9D 9E 93 F6 6F 78
E6 16 CD BA 69 AC 67 78 A3 AB C5 B4 73 1F 90 02 6A 7D B9 C4 B3 00 A3 31 6E 85 BD
0B 45 84 4D 44 F1 94 71 4C 9C 97 57 71 D8 5F 86 47 6B 40 18 C5 14 EC 28 C2 07 E9
F1 1C 16 7C AC 93 3B 69 10 B9 4C E0 8A 39 C0

transmit for FEC mode 0 17 pdu bytes in 235 ms
alpdu:  00 00 10 0B 0B B8 44 5A EC 01 06 08 11 84 C9 11 04
encoded:  EB 90 B4 33 AA AA 35 2E F8 53 0D C5 D4 2F AE 54 25 9D 9E 93 F6 6F 78
E6 16 CD 83 CA 53 98 89 E8 13 AB F1 34 EE 4C E0 B0 EB 96 42 1F 15 7F A2 8A 12 96
81 97 E8 36 4A 7F 35 E9 D9 D0 A5 E5 EC 72 7B CE 9F 28 61 01 7B 8B 22 F2 59 06 23
B0 72 76 D7 2C A0 9F EB D0 B7 C2 9D A7 D5 C0

*/
#include "correct.h"
#include "correct/convolutional/convolutional.h"
#include "encode.h"
#include "printers.h"
#include "tea.h"
#include <limits.h>
#include <string.h>

void create_reverse_table();

// typedef Short distance_t;
// static const distance_t distance_max = UINT16_MAX;

#define BSYNC_SIZE 6
#define FSYNC_SIZE 4
#define BIT_SYNC 0xEB, 0x90, 0xB4, 0x33, 0xAA, 0xAA
#define FS_MODE0 0x35, 0x2E, 0xF8, 0x53
#define FS_MODE1 0x58, 0x98, 0x23, 0x3E
#define FS_MODE2 0xEE, 0x43, 0x4E, 0x88

// encoding:
// FEC mode     0       1       2
// poly's       5B 79   1EB 171 1EB 171
// window       7       9       9
// code puncher 1/1     2/3     3/4

static Byte alpdu[] = {0x00, 0x00, 0x10, 0x0B, 0x0B, 0xB8, 0x44, 0x5A, 0xEC,
                       0x01, 0x06, 0x08, 0x11, 0x84, 0xC9, 0x11, 0x04};
static Byte convolved[sizeof(alpdu) * 2 + 2];
static Short nbytes;

#define V27POLYA 0x6D // 0x5B but 7 lower bits reversed
#define V27POLYB 0x4F // 0x79 but 7 lower bits reversed

static void encode() {
    print("\nALPDU: "), printDec(sizeof(alpdu)), hbytes(alpdu, sizeof(alpdu));
    nbytes = convolve_bytes(alpdu, convolved, sizeof(alpdu));
    print("\nConvo: "), printDec(nbytes);
    hbytes(convolved, nbytes);
}

// decoding
// table has numstates rows
// each row contains all of the polynomial output bits concatenated together
// e.g. for rate 2, we have 2 bits in each row
// the first poly gets the LEAST significant bit, last poly gets most
// significant
void fill_table_static(unsigned int rate, unsigned int order,
                       const polynomial_t *poly, unsigned int *table) {
    for (shift_register_t i = 0; i < 1 << order; i++) {
        unsigned int out = 0;
        unsigned int mask = 1;
        for (size_t j = 0; j < rate; j++) {
            out |= (popcount(i & poly[j]) % 2) ? mask : 0;
            mask <<= 1;
        }
        table[i] = out;
    }
}

#define MAX_ORDER 7
#define MAX_STATES (1<<MAX_ORDER)

history_buffer *history_buffer_create_static(unsigned int mtl, unsigned int tgl,
                                             unsigned int renormalize_interval,
                                             unsigned int num_states,
                                             shift_register_t highbit) {
#define min_traceback_lengthx (5 * MAX_ORDER)
#define traceback_group_lengthx (15 * MAX_ORDER)
#define capx (min_traceback_lengthx + traceback_group_lengthx)

    static history_buffer hb;
    memset(&hb, 0, sizeof(hb));
    history_buffer *buf = (void*)&hb;
    *(unsigned int *)&buf->min_traceback_length = min_traceback_lengthx;
    *(unsigned int *)&buf->traceback_group_length = traceback_group_lengthx;
    *(unsigned int *)&buf->cap = min_traceback_lengthx + traceback_group_lengthx;
    *(unsigned int *)&buf->num_states = num_states;
    *(shift_register_t *)&buf->highbit = highbit;

    if (mtl != min_traceback_lengthx || tgl != traceback_group_lengthx)
        print("\nhistory buffer error");

    static Byte bh[capx * sizeof(uint8_t *)], bf[capx * sizeof(uint8_t)];
    buf->history = (void*)bh;
    buf->fetched = (void*)bf;
    static Byte bhb[capx][MAX_STATES];
    memset(bhb, 0, sizeof(bhb));
    for (unsigned int i = 0; i < capx; i++)
        buf->history[i] = (void *)bhb[i];

    buf->index = 0;
    buf->len = 0;

    buf->renormalize_counter = 0;
    buf->renormalize_interval = renormalize_interval;

    return buf;
}

error_buffer_t *error_buffer_create_static(unsigned int num_states) {
    static error_buffer_t eb;
    error_buffer_t *buf = &eb;

    // how large are the error buffers?
    buf->num_states = num_states;

    // save two error metrics, one for last round and one for this
    // (double buffer)
    // the error metric is the aggregated number of bit errors found
    //   at a given path which terminates at a particular shift register state
    if (num_states > MAX_STATES) {
        print("\nError num_states is not 7: "), printDec(num_states);
    }
    static distance_t be1[1 << MAX_ORDER], be2[sizeof(be1)];
    memset(be1, 0, sizeof(be1));
    memset(be2, 0, sizeof(be2));
    buf->errors[0] = be1;
    buf->errors[1] = be2;

    // which buffer are we using, 0 or 1?
    buf->index = 0;

    buf->read_errors = buf->errors[0];
    buf->write_errors = buf->errors[1];

    return buf;
}

correct_convolutional *
correct_convolutional_create_static(size_t rate, size_t order,
                                    const polynomial_t *poly) {
    static correct_convolutional c;
    memset(&c, 0, sizeof(c));
    correct_convolutional *conv = &c;

    if (order > 8 * sizeof(shift_register_t)) {
        // XXX turn this into an error code
        print("order must be smaller than 8 * sizeof(shift_register_t)\n");
        return NULL;
    }
    if (rate < 2) {
        // XXX turn this into an error code
        print("rate must be 2 or greater\n");
        return NULL;
    }

    conv->order = order;
    conv->rate = rate;
    conv->numstates = 1 << order;

    static unsigned int t[1 << 9];
    unsigned int *table = t;
    fill_table_static(conv->rate, conv->order, poly, table);
    *(unsigned int **)&conv->table = table;

    static bit_writer_t bw;
    static bit_reader_t br;
    conv->bit_writer = &bw;
    conv->bit_reader = &br;

    conv->has_init_decode = true;

    static Byte d[(1 << (2)) * sizeof(distance_t)];

    conv->distances = (void *)d;
    conv->pair_lookup =
        pair_lookup_create(conv->rate, conv->order, conv->table);

    conv->soft_measurement = CORRECT_SOFT_LINEAR;

    uint64_t max_error_per_input = conv->rate * soft_max;
    unsigned int renormalize_interval = distance_max / max_error_per_input;

    // we limit history to go back as far as 5 * the order of our polynomial
    conv->history_buffer = history_buffer_create_static(
        5 * conv->order, 15 * conv->order, renormalize_interval,
        conv->numstates / 2, 1 << (conv->order - 1));

    conv->errors = error_buffer_create_static(conv->numstates);

    return &c;
}

static void decode() {
    Short *poly = (Short[]){V27POLYA, V27POLYB};
    correct_convolutional *conv =
        correct_convolutional_create_static(2, MAX_ORDER, poly);

    Byte decoded[1000];
    int n = correct_convolutional_decode(conv, convolved, nbytes * 8, decoded);
    print("\nDecod: ");
    printDec(n);
    if (n > 0)
        hbytes(decoded, min(n, 100));
}

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
    later(encode);
    later(decode);
    create_reverse_table();
}
