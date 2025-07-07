 rm -f a.out; \
 gcc -I ../libcorrect/include \
     -I ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/AirLink \
     -I ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/Encoder \
     -I ../Tea-host \
        main.c \
        printersHost.c \
        zero_crossing.c \
        tea.c \
        ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/Encoder/convolve012.c \
        ../libcorrect/src/convolutional/decode.c \
        ../libcorrect/src/convolutional/convolutional.c \
        ../libcorrect/util/error-sim.c \
        ../libcorrect/src/convolutional/bit.c \
        ../libcorrect/src/convolutional/encode.c \
        ../libcorrect/src/convolutional/error_buffer.c \
        ../libcorrect/src/convolutional/lookup.c \
        ../libcorrect/src/convolutional/history_buffer.c \
        ../libcorrect/src/convolutional/metric.c \
        ; ./a.out