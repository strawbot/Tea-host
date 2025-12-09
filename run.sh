 rm -f a.out; \
 gcc \
    -Ofast \
    -I ../Tea-host \
    -I ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/AirLink \
    -I ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/Encoder \
    -I ../AL200_Platforms/libcorrect/include \
    -I ../AL200_Platforms/libcorrect/include/correct/reed-solomon \
    main.c \
    tea.c \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/TimbreOS/printersHost.c \
    ../AL200_Platforms/AL200_OSX/Decoders/zero_crossings.c \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/Encoder/convolve012.c \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/Encoder/rs012.c \
    ../AL200_Platforms/AL200_OSX/Decoders/decode_static.c \
    ../AL200_Platforms/AL200_OSX/Decoders/correct_static.c \
    ../AL200_Platforms/libcorrect/src/convolutional/error_buffer.c \
    ../AL200_Platforms/libcorrect/src/convolutional/lookup.c \
    ../AL200_Platforms/libcorrect/src/convolutional/history_buffer.c \
    ../AL200_Platforms/libcorrect/src/convolutional/metric.c \
    ../AL200_Platforms/libcorrect/src/convolutional/bit.c \
    ../AL200_Platforms/libcorrect/src/convolutional/convolutional.c \
    ../AL200_Platforms/libcorrect/src/convolutional/decode.c \
    ../AL200_Platforms/libcorrect/src/reed-solomon/decode.c \
    ../AL200_Platforms/libcorrect/src/reed-solomon/polynomial.c \
    ../AL200_Platforms/libcorrect/src/reed-solomon/reed-solomon.c \
    && ./a.out
# ALPDU: 17  00 00 10 0B 0B B8 44 5A EC 01 06 08 11 84 C9 11 04
# Convo: 36  00 00 00 00 03 7C B0 D1 A2 A1 73 0D 6B FC FF 65 8B 3D 27 03 7C 8A 39 1F 2F 7F 13 AB 23 3B 53 CF CC 87 CB 00
# Decod: 17  00 00 10 0B 0B B8 44 5A EC 01 06 08 11 84 C9 11 04
# finished @ 0 ms  ups: 0
