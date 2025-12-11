 rm -f a.out; \
 gcc \
    -Wno-incompatible-pointer-types-discards-qualifiers \
    -I ../Tea-host \
    -I ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/Encoder \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/TimbreOS/printersHost.c \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/Encoder/rs012.c \
    ../Tea-host/parameters.c \
    ../Tea-host/rs_main1.c \
&& ls -l a.out \
    && ./a.out
