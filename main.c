#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osPRNG.h"

/* Possible improvements:
 * - Review usage of n_frame + frame_start vs frame_start + frame_end.
 * - Consider turning `frames` into an array of struct and put `is_dirty` and
 *   `references` from `struct entry` there.
 * - Employ X macros for statistics.
 * - Could use some unions to reuse less variables across algorithms.
 * - There is probably lots of refactoring opportunity.
 * - Zero attempts creating abstractions.
 */

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

static int frames[1024] = {0};
int n_frame = 0;
int frame_start = 0;
int pool_n_frame = 0;
int pool_frame_start = 0;
static struct link { struct link *prev, *next; int is_new; } links[1024] = {0};
struct link *new_start = NULL, *new_last = NULL,
            *old_start = NULL, *old_last = NULL;
int old_max, new_max, n_old = 0, n_new = 0;

/* This has gone too far. */

struct ll
{
    int n;
    struct link *first;
    struct link *last;
};

static struct ll old = {0};
static struct ll new = {0};

static void ll_put_at_0(struct ll *list, int frame_i);
static void ll_remove(struct ll *list, int i);
static int ll_remove_last(struct ll *list);

/**
 * If present:
 * 1. discover which list
 * 2. remove from it
 * 3. add to front of new list
 * If absent:
 * 1. choose from evicted or new
 * 2. add to front of old list
 */

static void usage(void);
static int ilog10(int);

int main(int argc, char *argv[])
{
    long frame_max;
    int page_width;
    int is_verbose = 0;
    enum alg alg = 0;
    unsigned long address;
    int memory_access_count = 0;
    int    page_fault_count = 0;
    int    disk_write_count = 0;
    int     disk_read_count = 0;

    atexit(osPRNG_deinit);

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

    old_max = frame_max * 2 / 5;
    new_max = frame_max - old_max;

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

    for (; 1 == scanf("%lx ", &address); memory_access_count++)
    {
        enum op op = getchar();
        unsigned long page_i = address >> page_width;

        if (op != OP_WRITE && op != OP_READ)
            break;

        if (ALG_LEAST_FREQUENTLY_USED == alg
			&& !(memory_access_count % 4) && n_frame > 0)
        {
            pool_n_frame++;
            frame_start = (frame_start + 1) % frame_max;
            n_frame--;
        }

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

            if (ALG_MIDPOINT_INSERTION == alg)
            {
                int frame_i = table[page_i].frame_i;
                if (!links[frame_i].is_new)
                {
                    /* Tira dos velhos. */
                    if (links[frame_i].prev)
                        links[frame_i].prev->next = links[frame_i].next;
                    if (links[frame_i].next)
                        links[frame_i].next->prev = links[frame_i].prev;
                    if (links + frame_i == old_start)
                        old_start = links[frame_i].next;
                    if (links + frame_i == old_last)
                        old_last = links[frame_i].prev;
                    n_old--;
                    n_new++;
                }

                /* Bota no início do new list. */
                links[frame_i].prev = NULL;
                links[frame_i].next = new_start;
                links[frame_i].is_new = 1;
                new_start = links + frame_i;
                if (NULL == new_last)
                    new_last = links + frame_i;

                if (links[frame_i].is_new && n_new - 1 == new_max)
                {
                    if (new_last != NULL)
                    {
                        /* Bota último new na lista de velhos.*/
                        if (new_last->prev)
                            new_last->prev->next = NULL;
                        new_last->next = old_start;
                        new_last->is_new = 0;
                        if (old_start && old_start->next)
                            old_start->next->prev = new_last;
                        old_start = new_last;

                        n_old++;
                        assert(n_old <= old_max);
                    }
                    n_new--;
                }
            }
        }
        else
        {
            int frame_i;

            page_fault_count++;

            if (ALG_MIDPOINT_INSERTION == alg)
            {
                int must_send_me_to_hell = 0;

                if (n_old < old_max)
                    frame_i = n_old++ + n_new;
                else
                {
                    frame_i = old_last - links;

                    if (old_last && old_last->prev)
                        old_last->prev->next = NULL;
                    if (old_last)
                        old_last = old_last->prev;

                    must_send_me_to_hell = 1;
                }

                /*Adiciona no início dos velhos.*/

                links[frame_i].prev = NULL;
                links[frame_i].next = old_start;
                links[frame_i].is_new = 0;

                old_start = links + frame_i;

                if (!old_last)
                    old_last = old_start;

                if (must_send_me_to_hell)
                    goto send_me_to_hell_idc;
            }
            else if (pool_n_frame + n_frame < frame_max)
                frame_i = frame_start + n_frame++;
            else
            {
                switch (alg)
                {
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

                    case ALG_LEAST_FREQUENTLY_USED:
                        if (0 == pool_n_frame)
                        {
                            frame_i = frame_start;
                            frame_start = (frame_start + 1) % frame_max;
                        }
                        else
                        {
                            int i;
                            int best=table[frames[pool_frame_start]].references;
                            frame_i = pool_frame_start;
                            for (
                                i = (pool_frame_start + 1) % frame_max;
                                i < (pool_frame_start+pool_n_frame) % frame_max;
                                i = (i + 1) % frame_max
                            ) {
                                int candidate = table[frames[i]].references;
                                if (candidate < best)
                                {
                                    best = candidate;
                                    frame_i = i;
                                }
                            }

                            /* Bota o primeiro do pool no lugar do escolhido. */
                            frames[frame_i] = frames[pool_frame_start];
                            frame_i = pool_frame_start;

                            /* No lugar do primeiro, bota a página nova. */
                            pool_frame_start = (pool_frame_start+1) % frame_max;
                            pool_n_frame--;
                            n_frame = (n_frame + 1) % frame_max;
                        }
                    break;

                    default:
                        assert(!"Unhandled page replacement algorithm.");
                    break;
                }
send_me_to_hell_idc:

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

/* Naïve solution. TODO */
static int ilog10(int n)
{
    int res = 0;
    for (; n >= 10; res++)
        n /= 10;
    return res;
}
