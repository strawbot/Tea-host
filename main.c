// Tea testing
#include "tea.h"
#include "encode.h"
#include "printers.h"
#include "parameters.h"
#include <string.h>
#include <stdlib.h>

#define view 80

void init_decoder();
void test_mls();
void test_rs012();
void test_convo();
void init_zc();
void serve_tea();
Long bit_sequenced(Byte * frame, Short n, Byte * bits);
Long find_bit_sync(Byte * bits);
Long find_frame_sync(Byte * bits);
Short bits_to_bytes(Byte * bits, Byte * bytes, Short len);
int deconvolve_buffer(Byte * frame, Byte * decoded, Short length);
int decode_rs(Byte * data, Long size);
Byte * trim_bits(Byte * seq, Short bits);
void frame_test();
void fec2_bits(Long n);

// Short count_bits(Byte * bits, Short nbits);
// Short match_bit_seqs(Byte * sync, Byte * frame);

int main() {
    init_tea();
    init_zc(); // from init_comps
    init_decoder();
    // later(test_mls);
    // later(test_rs012);
    // later(test_convo);
    later(frame_test);
    fec2_bits(74);
    serve_tea();
    return 0;
}

// static char *allPp[] = {    // GF_BITS  polynomial      */
// NULL,           /*  0   no code         */
// NULL,           /*  1   no code         */
// "111",          /*  2   1+x+x^2                          0x07*/ 
// "1101",         /*  3   1+x+x^3                          0x0B*/
// "1100 1",            /*  4   1+x+x^4                     0x13*/
// "1010 01",           /*  5   1+x^2+x^5                   0x25*/
// "1100 001",          /*  6   1+x+x^6                     0x43*/
// "1001 0001",         /*  7   1 + x^3 + x^7               0x89*/
// "1011 1000 1",        /*  8   1+x^2+x^3+x^4+x^8         0x11D*/
// "1000 1000 01",       /*  9   1+x^4+x^9                 0x211*/
// "1001 0000 001",      /* 10   1+x^3+x^10                0x409*/
// "1010 0000 0001",     /* 11   1+x^2+x^11                0x805*/
// "1100 1010 0000 1",        /* 12   1+x+x^4+x^6+x^12    0x1053*/
// "1101 1000 0000 01",       /* 13   1+x+x^3+x^4+x^13    0x201B*/
// "1100 0010 0010 001",      /* 14   1+x+x^6+x^10+x^14   0x4443*/
// "1100 0000 0000 0001",     /* 15   1+x+x^15            0x8003*/
// "1101 0000 0000 1000 1"     /* 16   1+x+x^3+x^12+x^16 0x1100B*/
// };
// 0001 1000 0111
// 1100 0011