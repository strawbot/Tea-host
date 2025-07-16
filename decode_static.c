#include "decode_static.h"
#include "printers.h"
#include <string.h>

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
