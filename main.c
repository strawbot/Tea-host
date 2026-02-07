// Tea testing
#include "tea.h"
#include "encode.h"
#include "printers.h"
#include "parameters.h"
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
Short bits_to_bytes(Byte * bits, Byte * bytes, Short len);
Short puncbits_to_bytes(Byte * bits, Byte * bytes, Short nbits, Byte mode);
int deconvolve_buffer(Byte * frame, Byte * decoded, Short length);
int decode_rs(Byte * data, Long size);
Short count_bits(Byte * bits, Short nbits);
Byte * trim_bits(Byte * seq, Short bits);
Short match_bit_seqs(Byte * sync, Byte * frame);

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
      
#define TEST_PDU alpdu1

extern Byte bit_sync_bits[BSYNC_SIZE * 8];
extern Byte frame_sync_bits[3][FSYNC_SIZE * 8];
#define view 80

void continue_test() {
    AirFrame * afe = get_airframe();
    Byte bits[8 * afe->nbytes];
    afe->bits = bits;
    afe->nbits = frame_bits(afe->bytes, afe->nbytes, afe->bits); // nbits is number of sequences
    print("\n Encoded: "), printDec(afe->nbytes), print("bytes  "), hbytes(afe->bytes, min(30, afe->nbytes));
    print("\n Encoded: "), printDec(afe->nbits), print("bits  "), hbytes(afe->bits, min(30, afe->nbits));
    print("\nDecoding: ");
    Short b;
    printDec((b = count_bits(afe->bits, afe->nbits))), print("bits  ");
    printDec(b/8), print("bytes");

    // Byte temp[b/8]; // maybe too large if agc is included; need to get to pipe-block model
    // Short n = bits_to_bytes(bits, temp, afe->nbits);
    // print("\nRaw bytes: "), hbytes(temp, min(n,30));

    print("\n Find bit sync:");
    AirFrame afd = {.bits = afe->bits, .nbits = afe->nbits};
    Long offset = find_bit_sync(afd.bits - 1);
    print(" offset = "), printDec(offset);
    if (offset >= afd.nbits) {
        print("  No bit sync match ");
        return;
    }
    // print("\n Bitsyncbits:"),hbytes(bit_sync_bits,strlen((char *)bit_sync_bits));
    // print("\n Framesyncbits:"),hbytes(frame_sync_bits[0],strlen((char *)frame_sync_bits[0]));
    // print("\nBits: "), hbytes(bits, view);
    // print("\nafd.Bits: "), hbytes(afd.bits, view);
    afd.bits = trim_bits(afd.bits, offset);
    afd.nbits -= offset; // recalculate length after removing syncs
    // print("\nafd.Bits: "), hbytes(afd.bits, view);
    // print("\nBits: "), hbytes(bits, view);

    // afd.bits = trim_bits(afd.bits, FSYNC_SIZE * 8);
    // print("\nafd.Bits: "), hbytes(afd.bits, view);
    // print("\nBits: "), hbytes(bits, view);
    Byte bytes[afd.nbits+1];
    afd.bytes = bytes;
    afd.nbytes = bits_to_bytes(afd.bits, afd.bytes, afd.nbits);
    // afd.nbytes = puncbits_to_bytes(afd.bits, afd.bytes, afd.nbits, fecmode);
    print("\n Bits to Bytes: "); 
    printDec(afd.nbytes), print("bytes "); hbytes(afd.bytes, min(30, afd.nbytes));
    // print("\nInject 8 errors");
    // for (Byte i = 0; i < 9; i++)
    //     afd.bytes[10 + afd.nbytes/8 + i] ^= 0xFF; // inject error

    Byte fecmode = find_frame_sync(afd.bytes + BSYNC_SIZE);
    if (fecmode > 2) { print("\n Cannot decode further! Bad FEC mode."); return; }

    print("\n FEC mode: ");
    printDec(fecmode);

    print("\n Split into segments: ");
    afd.nrs = fecmode == 0 ? FEC0_NROOTS : FEC12_NROOTS; // nroots
    afd.seg1 = afd.bytes + SYNC_SIZE;
    afd.nseg1 = fecmode == 0 ? SEG10_SIZE : fecmode == 1 ? SEG11_SIZE : SEG12_SIZE;
    afd.nseg2 = afd.nbytes > afd.nseg1 ? afd.nbytes - SYNC_SIZE - afd.nseg1 : 0;
    Byte seg2[afd.nseg2 ? afd.nseg2 : 1];
    afd.seg2 = afd.nseg2 ? afd.seg1 + afd.nseg1 : NULL;
    printDec((afd.seg1 != NULL) + (afd.seg2 != NULL));
    printDec(afd.nseg1 + afd.nseg2), print("bytes  ");

    print("\n Deconvolve segment 1: "); // nseg1 length is for data only
    Byte seg1[afd.nseg1/2];
    printDec(afd.nseg1), print("bytes  ");
    int n = deconvolve_buffer(afd.seg1, seg1, afd.nseg1*8) - afd.nrs;
    if (n < 0) { print("\n Faild deconvolving"); return; }
    afd.nseg1 = n;
    // print("  NSEG1 = "), printDec(afd.nseg1);
    print("\n  Decode errors in block 1: ");
    printDec(decode_rs(seg1, afd.nseg1));
    print("\n  Descramble block 1: ");
    unscramble_blk(seg1, afd.nseg1);
    hbytes(seg1, afd.nseg1);

    // Assemble block 1
    Short len = seg1[0] << 8 | seg1[1];
    if (len > 1023) { print("\n Length is too big, aborting decoding"); return; }
    Byte airpdu[len];
    Short nseg1 = min(len, afd.nseg1 - 2);
    memcpy(airpdu, seg1 + 2, nseg1);

    if (afd.nseg2) {
        print("\n Deconvolve segment 2: ");
        printDec(afd.nseg2), print("bytes  ");
        
        print("\n  Decode errors in blocks: ");
        n = deconvolve_buffer(afd.seg2, seg2, afd.nseg2*8);
        if (n < 0) { print("\n Faild deconvolving"); return; }
        afd.nseg2 = n;
        Byte blockm = fecmode == 0 ? BLOCKm0 : fecmode == 1 ? BLOCKm1 : BLOCKm2;
        Byte rsblock = blockm + afd.nrs;
        for (Short s = 0, d = 0; s < afd.nseg2; s += rsblock, d += blockm) {
            int n = min(rsblock, afd.nseg2 - s) - afd.nrs;
            if (n > 0) {
                printDec(decode_rs(seg2 + s, n));
                unscramble_blk(seg2 + s, n);
                memcpy(airpdu + nseg1 + d, seg2 + s, n);
            } else
                printDec(n);
        }
    }

    print("\n Assemble blocks: ");
    print("\n airpdu:  ");
    printDec(len), print("bytes  "), hbytes(airpdu, min(40, len));

    if (len == sizeof(TEST_PDU) && memcmp(airpdu, TEST_PDU, sizeof(TEST_PDU)) == 0)
        print("  Matched ");
    else
        print("  Different! ");
    printCr();
}

void frame_test() {
    print("Encode an airlink pdu for decoding testing:");
    print("\n airpdu:  "), printDec(sizeof(TEST_PDU));
    print("bytes  "), hbytes(TEST_PDU, min(40,sizeof(TEST_PDU)));
    set_value(FEC_Mode, FEC_MODE0);
    encode_airpdu(TEST_PDU, sizeof(TEST_PDU));
    when(mants_encoded, continue_test);
}

int main() {
    init_tea();
    init_zc(); // from init_comps
    init_decoder();
    // later(test_mls);
    // later(test_rs012);
    later(test_convo);
    // later(frame_test);
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