 rm -f a.out; \
 gcc \
    -I ../Tea-host \
    -I ../AL200_Platforms/libcorrect/include \
    -I ../AL200_Platforms/libcorrect/include/correct/reed-solomon \
    -I ../AL200_Platforms/libcorrect/tests/include \
    rs_main.c \
    ../AL200_Platforms/libcorrect/src/reed-solomon/reed-solomon.c \
    ../AL200_Platforms/libcorrect/src/reed-solomon/polynomial.c \
    ../AL200_Platforms/libcorrect/tests/rs_tester.c \
    ../AL200_Platforms/libcorrect/src/reed-solomon/decode.c \
    ../AL200_Platforms/libcorrect/src/reed-solomon/encode.c \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/TimbreOS/printersHost.c \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/Encoder/rs012.c \
    ../AL200_Platforms/libfec/decode_rs.c \
    ../AL200_Platforms/libfec/init_rs.c \
&& ls -l a.out \
    && ./a.out
