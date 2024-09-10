#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osPRNG.h"

#define ALG_XS(X, SP)           \
X(fifo, FIRST_IN_FIRST_OUT   )SP \
X(gc  , GLOBAL_CLOCK         )SP \
X(lfu , LEAST_FREQUENTLY_USED)SP \
X(lru , LEAST_RECENTLY_USED  )SP \
X(rand, RANDOM               )SP \
X(mid , MIDPOINT_INSERTION   )

#define PAGE_SIZE_XS(X, SP) X(1024, 10)SP X(4096, 12)SP X(16384, 14)

#define AS_ENUM(X, Y) ALG_##Y
#define AS_STR(X, Y) #X
#define AS_CASE(X, Y) case X: page_width = Y; break;
#define NIL
#define COMMA ,

enum alg { ALG_XS(AS_ENUM, COMMA), N_ALG, ALG_NONE = N_ALG };
enum op {OP_WRITE = 'W', OP_READ = 'R'};

char *algs[] = { ALG_XS(AS_STR, COMMA) };

static struct entry
{
    int is_present;
    int is_dirty;
    int references;
    int has_something;
    int frame_i;
} table[1 << 22] = {0};

/* Rewrite as struct with optional members. */
static int frames[1024] = {0};
int n_frame = 0;
int frame_start = 0;

static void usage(void);
static int ilog10(int);

int main(int argc, char *argv[])
{
    long frame_max;
    int page_width;
    int is_verbose = 0;
    enum alg alg = 0;
    unsigned long address;
    char op;
    int memory_access_count = 0;
    int    page_fault_count = 0;
    int    disk_write_count = 0;
    int     disk_read_count = 0;

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

    frame_max = strtol(argv[1 + is_verbose], NULL, 10);

    if (frame_max <= 0 || frame_max > 1024)
    {
        fprintf(
            stderr,
            "Numero de paginas fisicas deve ser de 1 a 1024, forneceu: %s\n",
            argv[1 + is_verbose]
        );
        return EXIT_FAILURE;
    }

    page_width = strtoul(argv[2 + is_verbose], NULL, 10);

    if (ERANGE == errno)
        usage();

    switch (page_width)
    {
        PAGE_SIZE_XS(AS_CASE, NIL)
        default: usage(); break;
    }

    while (alg < N_ALG && strcmp(argv[3 + is_verbose], algs[alg]))
        alg++;

    if (ALG_NONE == alg)
        usage();

    /* Paginate. */

    for (; 2 == scanf("%lx %[RW]", &address, &op); memory_access_count++)
    {
        unsigned long page_i = address >> page_width;

        if (table[page_i].is_present)
        {
            table[page_i].references++;

            if (ALG_LEAST_RECENTLY_USED == alg)
            {
                int frame_last    = (frame_start - 1 + n_frame) % n_frame;
                int frame_current = table[page_i].frame_i;
                int frame_tmp     = frames[frame_last];
                frames[frame_last] = frames[frame_current];
                frames[frame_current] = frame_tmp;
            }
        }
        else
        {
            int frame_i;

            page_fault_count++;

            if (n_frame < frame_max)
                frame_i = n_frame++;
            else
            {
                switch (alg)
                {
                    case ALG_LEAST_FREQUENTLY_USED: /* Fall through. XXX Temporary! */
                    case ALG_RANDOM:
                        frame_i = osPRNG() % frame_max;
                    break;

                    case ALG_LEAST_RECENTLY_USED: /* Fall through. */
                    case ALG_FIRST_IN_FIRST_OUT:
                        frame_i = frame_start; 
                        frame_start = (frame_start + 1) % frame_max; 
                    break;

                    case ALG_GLOBAL_CLOCK:
                        while (table[frames[frame_start]].references)
                        {
                            table[frames[frame_start]].references = 0;
                            frame_start = (frame_start + 1) % frame_max;
                        }
                        frame_i = frame_start;
                        frame_start = (frame_start + 1) % frame_max; 
                    break;

                    default:
                        assert(!"Unknown page replacement algorithm.");
                    break;
                }

                if (table[frames[frame_i]].is_dirty)
                    disk_write_count++;

                table[frames[frame_i]].is_present = 0;

		frames[frame_i] = page_i;
            }

            if (table[page_i].has_something)
                disk_read_count++;

            table[page_i].is_present = 1;
            table[page_i].is_dirty   = 0;
            table[page_i].references = 0;
            table[page_i].frame_i    = frame_i;
        }

        if (OP_WRITE == op)
            table[page_i].has_something = table[page_i].is_dirty = 1;
    }

    /* Statistics. */

    {
        int width = memory_access_count;
        if (page_fault_count > width) width = page_fault_count;
        if (disk_write_count > width) width = disk_write_count;
        if (disk_read_count  > width) width =  disk_read_count;
        width = ilog10(width) + 1;

        printf(
            "Numero de acessos a memoria: %*d\n"
            "Numero de faltas de pagina:  %*d\n"
            "Numero de escritas em disco: %*d\n"
            "Numero de leituras em disco: %*d\n",
            width, memory_access_count,
            width, page_fault_count,
            width, disk_write_count,
            width, disk_read_count
       );
    }

    return 0;
}

static void usage(void)
{
    fprintf(stderr, "Uso: simulador [-v] <numero_de_paginas_fisicas> "
            PAGE_SIZE_XS(AS_STR, "|") " "
                  ALG_XS(AS_STR, "|") "\n");
    exit(EXIT_FAILURE);
}

/* Naïve solution. */
static int ilog10(int n)
{
    int res = 0;
    for (; n >= 10; res++)
        n /= 10;
    return res;
}
