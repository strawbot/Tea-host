#include "correct.h"
#include "correct/convolutional/convolutional.h"

#ifndef DECODE_STATIC_H
#define DECODE_STATIC_H

#define RATE 2
#define MAX_ORDER 7
#define MAX_STATES (1<<MAX_ORDER)
#define min_traceback_lengthx (5 * MAX_ORDER)
#define traceback_group_lengthx (15 * MAX_ORDER)
#define capx (min_traceback_lengthx + traceback_group_lengthx)

void fill_table_static(unsigned int rate, unsigned int order,
                       const polynomial_t *poly, unsigned int *table);

pair_lookup_t pair_lookup_create_static(unsigned int rate, unsigned int order,
                                 const unsigned int *table);

history_buffer *history_buffer_create_static(unsigned int mtl, unsigned int tgl,
                                             unsigned int renormalize_interval,
                                             unsigned int num_states,
                                             shift_register_t highbit);
error_buffer_t *error_buffer_create_static(unsigned int num_states);

correct_convolutional *
correct_convolutional_create_static(size_t rate, size_t order, const polynomial_t *poly);

#endif