// Tea testing
#include "tea.h"
#include "encode.h"
#include "printers.h"
#include <string.h>

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
Byte bits_to_bytes(Byte * bits, Byte * bytes, Short len);
int decode_buffer(Byte * frame, Byte * decoded, Short length);
int decode_rs(Byte * data, Long size);
Short count_bits(Byte * bits, Short nbits);

static Byte alpdu1[] = {0x00, 0x00, 0x10, 0x0B, 0x0B, 0xB8, 0x44, 0x5A, 0xEC,
                        0x01, 0x06, 0x08, 0x11, 0x84, 0xC9, 0x11, 0x04};
static Byte alpdu2[] = {0x04, 0x00, 0x10, 0x16, 0x03, 0xE8, 0x00, 0x02, 0x04,
                        0x00, 0x12, 0x00, 0x00, 0x01, 0x09, 0x08, 0x11, 0x03,
                        0x0A, 0x11, 0x00, 0x09, 0x11, 0x00, 0x05, 0x02, 0x20,
                        0x40};
static Byte alpdu3[] = {0x04, 0x00, 0x10, 0xD5, 0x03, 0xE8, 0x10, 0x02, 0x80, 0xC0, 0x00,
      0x12, 0x00, 0xBC, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x0F, 0x01, 0x34, 0x7F, 
      0xC0, 0x00, 0x00, 0x0A, 0x11, 0x00, 0x08, 0x11, 0x02, 0x09, 0x11, 0x00};
      
#define TEST_PDU alpdu3

void continue_test() {
    AirFrame * afe = get_airframe();
    Byte bits[8 * afe->nbytes];
    afe->bits = bits;
    afe->nbits = frame_bits(afe->bytes, afe->nbytes, afe->bits);
    print("\n Encoded: "), printDec(afe->nbits), hbytes(afe->bits, min(30, afe->nbits));
    print("\nDecoding: ");
    printDec(count_bits(afe->bits, afe->nbits)), print("bits");
    print("\n Find bit sync: ");
    AirFrame afd = {.bits = afe->bits, .nbits = afe->nbits};
    Long offset = find_bit_sync(afd.bits);
    print("  offset = "), printDec(offset);
    print("\n Bits to Bytes: "); 
    Byte bytes[afd.nbits+1];
    afd.bytes = bytes;
    afd.nbytes = bits_to_bytes(afd.bits + offset, afd.bytes, afd.nbits);
    printDec(afd.nbytes); hbytes(afd.bytes, min(30, afd.nbytes));

    print("\nInject 8 errors");
    for (Byte i = 0; i < 9; i++)
        afd.bytes[10 + i * 2] ^= 0xFF; // inject error

    print("\n FEC mode: ");
    Byte fecmode = find_frame_sync(afd.bytes + BSYNC_SIZE);
    printDec(fecmode);
    if (fecmode != 0) { print("\n Cannot decode further! Bad FEC mode."); return; }
    print("\n Split into segments: ");
    afd.nrs = FEC0_NROOTS; // nroots
    afd.seg1 = afd.bytes + SYNC_SIZE;
    afd.nseg1 = SEG1_SIZE;
    afd.nseg2 = afd.nbytes - SYNC_SIZE > afd.nseg1 ? afd.nbytes - SYNC_SIZE - afd.nseg1 : 0;
    Byte seg2[afd.nseg2 ? afd.nseg2 : 1];
    afd.seg2 = afd.nseg2 ? afd.seg1 + afd.nseg1 : NULL;
    printDec((afd.seg1 != NULL) + (afd.seg2 != NULL));
    printDec(afd.nseg1), print("bytes  ");
    print("\n Deconvolve segments: ");
    Byte seg1[afd.nseg1/2];
    print("\n  Seg1: "), printDec(afd.nseg1);
    decode_buffer(afd.seg1, seg1, afd.nseg1);
    if (afd.nseg2) {
        print("\n  Seg2: "), printDec(afd.nseg2);
        decode_buffer(afd.seg2, seg2, afd.nseg2);
        afd.nseg2 = (afd.nseg2 - CONV_TAIL)/2 - afd.nrs;
        decode_rs(seg2, afd.nseg2);
        unscramble_blk(seg2, afd.nseg2);
    }
    afd.nseg1 = (afd.nseg1 - CONV_TAIL)/2 - afd.nrs;
    // print("  NSEG1 = "), printDec(afd.nseg1);
    print("\n Decode RS blocks: ");
    printDec(decode_rs(seg1, afd.nseg1));
    hbytes(seg1, afd.nseg1);
    print("\n Descramble blocks: ");
    unscramble_blk(seg1, afd.nseg1);
    hbytes(seg1, afd.nseg1);
    print("\n Assemble blocks: ");
    print("\n airpdu:  ");
    Short len = seg1[0] << 8 | seg1[1];
    Short nseg1 = min(len, afd.nseg1 - 2);
    printDec(len), hbytes(seg1 + 2, nseg1); // start of airpdu
    hbytes(seg2, min(40-nseg1, afd.nseg2));

    Byte airpdu[len];
    memcpy(airpdu, seg1 + 2, nseg1);
    memcpy(airpdu + nseg1, seg2, afd.nseg2);
    if (len == sizeof(TEST_PDU) && memcmp(airpdu, TEST_PDU, sizeof(TEST_PDU)) == 0)
        print("  Matched ");
    else
        print("  Different! ");
    printCr();
}

void frame_test() {
    print("Encode an airlink pdu for decoding testing:");
    print("\n airpdu:  "), printDec(sizeof(TEST_PDU)), hbytes(TEST_PDU, min(40,sizeof(TEST_PDU)));
    encode_airpdu(TEST_PDU, sizeof(TEST_PDU));
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