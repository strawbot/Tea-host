 rm -f a.out; \
 gcc \
    -Ofast \
    -I ../libcorrect/include \
    -I ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/AirLink \
    -I ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/Encoder \
    -I ../Tea-host \
    main.c \
    printersHost.c \
    zero_crossing.c \
    tea.c \
    decode_static.c \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/Encoder/convolve012.c \
    ../libcorrect/src/convolutional/error_buffer.c \
    ../libcorrect/src/convolutional/lookup.c \
    ../libcorrect/src/convolutional/history_buffer.c \
    ../libcorrect/src/convolutional/metric.c \
    ../libcorrect/src/convolutional/bit.c \
    ../libcorrect/src/convolutional/convolutional.c \
    ../libcorrect/src/convolutional/decode.c \
    ; ls -l a.out \
    ; ./a.out
# ALPDU: 17  00 00 10 0B 0B B8 44 5A EC 01 06 08 11 84 C9 11 04
# Convo: 36  00 00 00 00 03 7C B0 D1 A2 A1 73 0D 6B FC FF 65 8B 3D 27 03 7C 8A 39 1F 2F 7F 13 AB 23 3B 53 CF CC 87 CB 00
# Decod: 17  00 00 10 0B 0B B8 44 5A EC 01 06 08 11 84 C9 11 04
# finished @ 0 ms  ups: 0
