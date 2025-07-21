 rm -f a.out; \
 gcc \
    -Ofast \
    -I ../Tea-host \
    -I ../libcorrect/include \
    -I ../libcorrect/include/correct/reed-solomon \
    -I ../libcorrect/tests/include \
    ../libcorrect/tests/reed-solomon.c \
    ../libcorrect/src/reed-solomon/reed-solomon.c \
    ../libcorrect/src/reed-solomon/polynomial.c \
    ../libcorrect/tests/rs_tester.c \
    ../libcorrect/src/reed-solomon/decode.c \
    ../libcorrect/src/reed-solomon/encode.c \
    && ls -l a.out \
    && ./a.out
