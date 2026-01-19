// Tea testing
#include "tea.h"
#include "encode.h"
#include "printers.h"

void init_zc();
void init_decoder();
void test_mls();
void test_rs012();
void test_convo();
void init_zc();
void serve_tea();
Long frame_bits(Byte * frame, Short n, Byte * bits);
Long find_bit_sync(Byte * bits);
Long find_frame_sync(Byte * bits);
Byte bits_to_bytes(Byte * bits, Byte * bytes);
int decode_buffer(Byte * frame, Byte * decoded, Short length);
int decode_rs(Byte * data, Long size);

static Byte alpdu[] = {0x00, 0x00, 0x10, 0x0B, 0x0B, 0xB8, 0x44, 0x5A, 0xEC,
                       0x01, 0x06, 0x08, 0x11, 0x84, 0xC9, 0x11, 0x04};

void continue_test() {
    AirFrame * afe = get_airframe();
    Byte bits[8 * afe->nbytes];
    afe->bits = bits;
    afe->nbits = frame_bits(afe->bytes, afe->nbytes, afe->bits);
    print("\n Encoded: "), printDec(afe->nbits), hbytes(afe->bits, min(30, afe->nbits));

    print("\nDecoding:");
    AirFrame afd = {.bits = afe->bits};
    Long offset = find_bit_sync(afd.bits);
    print("\n Find bit sync: ");
    print(" offset = "), printDec(offset);
    print("\n Bits to Bytes: "); 
    Byte bytes[afd.nbits+1];
    afd.bytes = bytes;
    afd.nbytes = bits_to_bytes(afd.bits + offset, afd.bytes);
    printDec(afd.nbytes); hbytes(afd.bytes, min(30, afd.nbytes));
    print("\n FEC mode: ");
    Byte fecmode = find_frame_sync(afd.bytes + BSYNC_SIZE);
    printDec(fecmode);
    if (fecmode != 0) { print("\n Cannot decode further! Bad FEC mode."); return; }
    print("\n Split into segments: ");
    afd.seg1 = afd.bytes + SYNC_SIZE;
    afd.nseg1 = SEG1_SIZE;
    afd.nseg2 = afd.nbytes - SYNC_SIZE > afd.nseg1 ? afd.nbytes - SYNC_SIZE - afd.nseg1 : 0;
    afd.seg2 = afd.nseg2 ? afd.seg1 + afd.nseg1 : NULL;
    printDec((afd.seg1 != NULL) + (afd.seg2 != NULL));
    print("\n Deconvolve segments: ");
    Byte seg1[afd.nseg1];
    print("\n  Seg1: ");
    decode_buffer(afd.seg1, seg1, afd.nseg1);
    if (afd.nseg2) {
        Byte seg2[afd.nseg2];
        print("\n  Seg2: "), decode_buffer(afd.seg2, seg2, afd.nseg2);
    }
    print("\n Decode RS blocks: ");
      printDec(decode_rs(seg1, afd.nseg1));
    print("\n Descramble blocks: ");
    print("\n Assemble blocks: ");
}

void frame_test() {
    print("Encode an airlink pdu for decoding testing:");
    print("\n airpdu:  "), printDec(sizeof(alpdu)), hbytes(alpdu, sizeof(alpdu));
    encode_airpdu(alpdu, sizeof(alpdu));
    when(mants_encoded, continue_test);
}

int main() {
    init_tea();
    init_zc(); // from init_comps
    init_decoder();
    // later(test_mls);
    // later(test_rs012);
    // later(test_convo);
    later(frame_test);
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