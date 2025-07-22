 rm -f a.out; \
 gcc \
    -Ofast \
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
    && ls -l a.out \
    && ./a.out
