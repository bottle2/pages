#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALG_XS(X, SP)  \
X(fifo, FIFO        )SP \
X(gc  , GLOBAL_CLOCK)SP \
X(lfu , LFU         )SP \
X(lru , LRU         )SP \
X(rand, RANDOM      )SP \
X(usr , USR         )

#define AS_ENUM(X, Y) ALG_##Y
#define AS_STR(X, Y) #X
#define NIL
#define COMMA ,

enum alg { ALG_XS(AS_ENUM, COMMA), N_ALG, ALG_NONE = N_ALG };
char *algs[] = { ALG_XS(AS_STR, COMMA) };

static void usage(char prog[])
{
    fprintf( stderr, "Uso: %s [-v] <numero_de_paginas> <tamanho_da_pagina> "
            ALG_XS(AS_STR, "|") "\n", prog);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int page_amount;
    int page_size;
    int is_verbose = 0;
    enum alg alg = 0;
    int address;
    enum op {OP_W = 'W', OP_R = 'R'} op;

    if (argc < 4 || argc > 5)
        usage(argv[0]);
    
    if (5 == argc)
    {
        if ('-' == argv[1][0] && 'v' == argv[1][1])
            is_verbose = 1;
        else
        {
            fprintf(stderr, "Esperava flag -v, passou: %s\n", argv[1]);
            return EXIT_FAILURE;
        }
    }

    page_amount = atoi(argv[1 + is_verbose]);
    page_size   = atoi(argv[2 + is_verbose]);

    while (alg < N_ALG && strcmp(argv[3 + is_verbose], algs[alg]))
        alg++;

    if (ALG_NONE == alg)
        usage(argv[0]);

    while (2 == scanf("%x %[WR]\n", &address, &op))
    {
    }

    return 0;
}
