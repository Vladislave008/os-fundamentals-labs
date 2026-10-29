#ifndef LAB3_POOL_H
#define LAB3_POOL_H

#include <pthread.h>
#include <stddef.h>

#define POOL_MAX_THREADS 1024u

typedef struct
{
    int *src;
    int *dst;

    int *a;
    int *b;

    size_t rows;
    size_t cols;
    size_t window;
    size_t iters;
    size_t block_rows;
    size_t total_blocks;

    pthread_mutex_t mutex;
    pthread_barrier_t barrier;

    size_t next_block;
    double elapsed;
} filter_job;

int filter_job_init(filter_job *job, int *src, int *dst, size_t rows, size_t cols,
                    size_t window, size_t iters, size_t block_rows);

void filter_job_destroy(filter_job *job);

void filter_job_set_block_rows(filter_job *job, size_t block_rows);

int filter_run(filter_job *job, size_t threads);

int filter_run_serial(filter_job *job);

#endif
