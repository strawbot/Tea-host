#include "decode_static.h"
#include "tea.h"
#include "printers.h"
#include <string.h>
#include "encode.h"

void create_reverse_table();

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
        for (size_t j = 0; j < RATE; j++) {
            out |= (popcount(i & poly[j]) % 2) ? mask : 0;
            mask <<= 1;
        }
        table[i] = out;
    }
}

pair_lookup_t pair_lookup_create_static(unsigned int rate, unsigned int order,
                                 const unsigned int *table) {
    pair_lookup_t pairs;

    static Byte pk[sizeof(unsigned int) * (1 << (MAX_ORDER - 1))];
    pairs.keys = (void *)pk;
    static Byte po[1 << (RATE * 2)][sizeof(unsigned int)];
    memset(po, 0, sizeof(po));
    pairs.outputs = (void *)po;
    Byte io[1 << (RATE * 2)][sizeof(unsigned int)];
    memset(io, 0, sizeof(io));
    unsigned int *inv_outputs = (void *)io;
    unsigned int output_counter = 1;
    // for every (even-numbered) shift register state, find the concatenated
    // output of the state
    //   and the subsequent state that follows it (low bit set). then, check to
    //   see if this concatenated output has a unique key assigned to it
    //   already. if not, give it a key. if it does, retrieve the key. assign
    //   this key to the shift register state.
    for (unsigned int i = 0; i < (1 << (order - 1)); i++) {
        // first get the concatenated pair of outputs
        unsigned int out = table[i * 2 + 1];
        out <<= RATE;
        out |= table[i * 2];

        // does this concatenated output exist in the outputs table yet?
        if (!inv_outputs[out]) {
            // doesn't exist, allocate a new key
            inv_outputs[out] = output_counter;
            pairs.outputs[output_counter] = out;
            output_counter++;
        }
        // set the opaque key for the ith shift register state to the
        // concatenated output entry
        pairs.keys[i] = inv_outputs[out];
    }
    if (output_counter != 5)
        print("\nDifferent output_coutner!  "), printDec(output_counter);

    pairs.outputs_len = output_counter;
    pairs.output_mask = (1 << (RATE)) - 1;
    pairs.output_width = RATE;
    static Byte pd[5][sizeof(distance_pair_t)]; // 5 is 
    memset(pd, 0, sizeof(pd));
    pairs.distances = (void *)pd;
    return pairs;
}

history_buffer *history_buffer_create_static(unsigned int mtl, unsigned int tgl,
                                             unsigned int renormalize_interval,
                                             unsigned int num_states,
                                             shift_register_t highbit) {
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
    if (RATE < 2) {
        // XXX turn this into an error code
        print("rate must be 2 or greater\n");
        return NULL;
    }

    conv->order = order;
    conv->rate = RATE;
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
    pair_lookup_create_static(conv->rate, conv->order, conv->table);

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

// test functions
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

static void decode() {
    Short *poly = (Short[]){V27POLYA, V27POLYB};
    correct_convolutional *conv =
        correct_convolutional_create_static(RATE, MAX_ORDER, poly);
    print("\nDecod: ");
    flush();
    Byte decoded[10000];
    int n = correct_convolutional_decode(conv, convolved, nbytes * 8, decoded);
    printDec(n);
    if (n > 0)
        hbytes(decoded, min(n, 100));
    print(memcmp(alpdu, decoded, n) == 0 ? " Match" : "Different");
}

void init_decoder() {
    create_reverse_table();
    later(encode);
    later(decode);
}

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
