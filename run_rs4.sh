 rm -f a.out; \
 gcc -O3 \
    -I ../Tea-host \
    -I "../AL200_Platforms/libcorrect/tests/include" \
    -I "../AL200_Platforms/libcorrect/include" \
    -I "../AL200_Platforms/AL200_OS6/Alert2Encoder/Unit_tests" \
    --include=ttypes.h \
    --include=fec_shim.h \
    ../AL200_Platforms/libcorrect/tests/reed-solomon-fec-interop.c \
    ../AL200_Platforms/libcorrect/tests/rs_tester.c \
    ../AL200_Platforms/libcorrect/tests/rs_tester_fec.c \
    ../AL200_Platforms/libcorrect/src/fec_shim.c \
    ../AL200_Platforms/libcorrect/src/reed-solomon/encode.c \
    ../AL200_Platforms/libcorrect/src/reed-solomon/decode.c \
    ../AL200_Platforms/libcorrect/src/reed-solomon/polynomial.c \
    ../AL200_Platforms/libcorrect/src/convolutional/convolutional.c \
    ../AL200_Platforms/libcorrect/src/convolutional/bit.c \
    ../AL200_Platforms/libcorrect/src/convolutional/decode.c \
    ../AL200_Platforms/libcorrect/src/convolutional/error_buffer.c \
    ../AL200_Platforms/libcorrect/src/convolutional/lookup.c \
    ../AL200_Platforms/libcorrect/src/convolutional/history_buffer.c \
    ../AL200_Platforms/libcorrect/src/convolutional/metric.c \
    ../AL200_Platforms/libcorrect/src/reed-solomon/reed-solomon.c \
    \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/TimbreOS/printersHost.c \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/Unit_tests/reedSolomen.c \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/Encoder/rs012.c \
    && ls -l a.out \
    && ./a.out
