#include "ttypes.h"
#include "sys/time.h"

#ifndef PROJECT_DEFS
#define PROJECT_DEFS

#define ONE_SECOND 16384		// RTC: 32K input/2; UTC

#define tx_debug() 0
#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)
Long get_ticks();
#define gps_data() true
#define devconfig_connected() false

int fec_debug(); // debug: 1 - print partial bits
#define FEC_DEBUG 0

// #define namedAction(action) 

#define sysTicks() clock()
#define SYS_TO_MS(t) (Long)((double)(t)*1000 / CLOCKS_PER_SEC)
#define SYS_TO_US(t) (Long)((double)(t)*1000*1000 / CLOCKS_PER_SEC)

#define ENTER_SAFE_REGION()
#define LEAVE_SAFE_REGION()

typedef long long ms_time;

typedef struct dcu_saved {
    Byte Operation_Mode;
    Byte Test_Mode;
    Byte Basic_Advanced;
    Byte Multi_Sensor_Report;
    Byte MSR_IND;
    Byte Mark_Space;
    Byte ALERT_Transmit_Message_Format;
    Byte Hold_off_Time;
    Short Preamble_Time;
    Byte Inter_packet_Spacing;
    Byte P1_Report;
    Short Sensor_Scan_Interval;
    float Sensor_Scan_Offset;
    Short Alert_Carrier;
    Short Alert_Tail;
    Long TBR_ZeroDate;
    Long TBR_Rollover;
    Byte TBR_ZeroSchedule;
} dcu_saved;

extern dcu_saved *dcu_settings;

bool alert2_sensors_mode();
bool sensor_mode();
bool ind_mode();
bool alert1_mode();
bool off_mode();
enum {
    NO_PROPERTY_TAG = 0,
    VERSION_TAG = 1,
    PENDING_KEY_TAG = 2,
    ACTIVE_KEY_TAG = 3,
    EMID_TAG = 4,
    SENSOR_TAG = 5,
    TIPS_TAG = 6,
    DEBUGS_TAG = 7,
    UTC_TAG = 8,
    NUM_PROPS
} property_tags;

#define write_file(fd, buf, len) false
#define cmox_initialize(pInitArg) NULL
#define CRC 0
#define LL_CRC_DEFAULT_CRC_INITVALUE       0xFFFFFFFFU
#define CMOX_AESFAST_CTR_ENC_ALGO 0
#define LL_CRC_SetInitialData(CRC, LL_CRC_DEFAULT_CRC_INITVALUE) (void)
#define cmox_cipher_encrypt(P_algo,P_pInput, P_inputLen, P_pKey, P_keyLen, P_pIv, P_ivLen, P_pOutput, P_pOutputLen) (void)
// encryption specifics
// #include "sha1.h"
// #include "sha2.h"
// #include "aes.h"

#define compute_sha1(headx, hlen, payload, plen, hash) (void)
    // sha1_context ctx; \
    // sha1_starts( &ctx ); \
    // sha1_update( &ctx, headx, hlen ); \
    // sha1_update( &ctx, payload, plen); \
    // sha1_finish( &ctx, hash )

#define compute_sha2(string, length, output) (void)
    // sha2_context ctx; \
    // sha2_starts( &ctx, 0); \
    // sha2_update( &ctx, string, length ); \
    // sha2_finish( &ctx, output )



#endif