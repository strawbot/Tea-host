 rm -f a.out; \
 gcc \
    -Wno-incompatible-pointer-types-discards-qualifiers \
    -I ../Tea-host \
    -I ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/Encoder \
    -I ../AL200_Platforms/libfec/ \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/TimbreOS/printersHost.c \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/Encoder/rs012.c \
    ../AL200_Platforms/libfec/init_rs.c \
    ../AL200_Platforms/libfec/init_rs_int.c \
    ../AL200_Platforms/libfec/init_rs_char.c \
    ../AL200_Platforms/libfec/rstest.c \
    ../AL200_Platforms/libfec/encode_rs_8.c \
    ../AL200_Platforms/libfec/encode_rs_char.c \
    ../AL200_Platforms/libfec/encode_rs_int.c \
    ../AL200_Platforms/libfec/decode_rs_8.c \
    ../AL200_Platforms/libfec/decode_rs_char.c \
    ../AL200_Platforms/libfec/decode_rs_int.c \
    ../AL200_Platforms/libfec/rs.c \
    ../Tea-host/parameters.c \
    ../Tea-host/decode_rs.c \
&& ls -l a.out \
    && ./a.out
