 rm -f a.out; \
 gcc \
    -Ofast \
    -I ../Tea-host \
    -I ../AL200_Platforms/AL200_OS6/Alert2Encoder/rscode-1.3 \
    -I "../AL200_Platforms/AL200_OS6/RS coders" \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/rscode-1.3/example.c \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/rscode-1.3/berlekamp.c \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/rscode-1.3/rs.c \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/rscode-1.3/galois.c \
    ../AL200_Platforms/AL200_OS6/Alert2Encoder/src/Encoder/rs012.c \
    && ls -l a.out \
    && ./a.out
