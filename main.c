#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO Differentiate virtual and physical address. */

#define ALG_XS(X, SP)        \
X(fifo, FIFO              )SP \
X(gc  , GLOBAL_CLOCK      )SP \
X(lfu , LFU               )SP \
X(lru , LRU               )SP \
X(rand, RANDOM            )SP \
X(mid , MIDPOINT_INSERTION)

#define AS_ENUM(X, Y) ALG_##Y
#define AS_STR(X, Y) #X
#define NIL
#define COMMA ,

enum op {OP_W = 'W', OP_R = 'R'};
enum alg { ALG_XS(AS_ENUM, COMMA), N_ALG, ALG_NONE = N_ALG };
char *algs[] = { ALG_XS(AS_STR, COMMA) };

static void usage(void)
{
    fprintf(stderr, "Uso: simulador [-v] <numero_de_paginas_fisicas> <tamanho_da_pagina> "
            ALG_XS(AS_STR, "|") "\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    long page_frame_amount;
    int page_size;
    int is_verbose = 0;
    enum alg alg = 0;
    unsigned long address;
    char op;
    int memory_access_amount = 0;
    int page_fault_amount = 0;
    int disk_write_amount = 0;
    int disk_read_amount = 0;

    /* Configuration. */

    if (argc < 4 || argc > 5)
        usage();
    
    if (5 == argc)
    {
        if ('-' == argv[1][0] && 'v' == argv[1][1])
            is_verbose = 1;
        else
        {
            fprintf(stderr, "Flag deveria ser -v, forneceu: %s\n", argv[1]);
            return EXIT_FAILURE;
        }
    }

    page_frame_amount = strtol(argv[1 + is_verbose], NULL, 10);

    if (page_frame_amount <= 0 || page_frame_amount > 1024)
    {
        fprintf(stderr,
                "Numero de paginas fisicas deve ser de 1 a 1024, forneceu: %s\n",
                argv[1 + is_verbose]);
        return EXIT_FAILURE;
    }

    page_size = strtoul(argv[2 + is_verbose], NULL, 10);

    if (ERANGE == errno || !(page_size & (page_size - 1)))
    {
        fprintf(stderr,
                "Tamanho da pagina deve ser potencia de dois de 1 a 2^31, forneceu: %s\n",
                argv[2 + is_verbose]);
        return EXIT_FAILURE;
    }

    while (alg < N_ALG && strcmp(argv[3 + is_verbose], algs[alg]))
        alg++;

    if (ALG_NONE == alg)
        usage();

    /* Paginate. */

    while (2 == scanf("%lx %[RW]", &address, &op))
    {
        unsigned long offset = address & (~address & (address - 1));
        unsigned long page_i = (address & (~address | (address + 1))) >> ;
    }

    /* Statistics. */

    {
        int width = 4;

        printf("Numero de acessos a memoria: %*d"
               "Numero de faltas de pagina:  %*d"
               "Numero de escritas em disco: %*d"
               "Numero de leituras em disco: %*d",
               width, 0, width, 0, width, 0, width, 0);
    }

    return 0;
}
