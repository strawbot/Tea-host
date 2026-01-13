#include "project_defs.h"
#include "printers.h"

Byte fecmode;

bool alert2_sensors_mode() { return true; }
bool sensor_mode() { return true; }
bool ind_mode() { return true; }
bool alert1_mode() { return true; }
bool off_mode() { return true; }

bool aloha() { return false; }

dcu_saved settings = {0};
dcu_saved *dcu_settings = &settings;

Byte encrypt_cost() { return 0; }
void encrypt_mant_pdu(Byte * head) { }
bool encryption_valid() { return false; }

static Byte base = 10; // command line number radix

#define SPACE 0x20

bool toDigit(Byte* n) { // convert character to number according to base
    Byte c = *n - '0'; // covers all alphanumerics and bases

    if (c > 9) {
        c -= 7;
        if (c > 35)
            c -= SPACE;
        if (c < 10)
            return false;
    }
    if (c >= base)
        return false;
    *n = c;
    return true;
}

Long hexscii_convert(Byte *hs, Byte *hex) { // "AABBCC" -> 0xAA,0xBB,0xCC
    // convert characters to hex nibbles till end found
    Byte wasbase = base;
    base = 16;
    Byte *end = hs;
    while (toDigit(end))  end++;
    base = wasbase;
    // fold nibbles into bytes till end
    Byte n = 0;
    for (; hs < end; hs += 2, n++)
        *hex++ = hs[0] << 4 | hs[1];
    return n;
}

void maybeCr() { print("\n"); }
Byte* parseWord(Byte c) { return (Byte *)""; }
void transmit_frame(Byte *frame, Short len) { print("\n Frame transmitted "); }
bool valid_slot() { return false; }

Long gps_interval() { return 1800; }

void init_tll();

void send_airpdu() { }

void set_utc(Long t) { }
 
void sync_utc_gps() { }

bool save_prop(Byte tag, void * a, Byte n) { return false; }
bool load_prop(Byte tag, void * a, Byte n) { return false; }
void * get_prop(Byte tag) { return NULL; }
