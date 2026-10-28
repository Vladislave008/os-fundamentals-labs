#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "matrix_filter.h"
#include "pool.h"
#include "util.h"

static int *alloc_ints(size_t n)
{
    int *data = (int *)malloc(n * sizeof(*data));
    if (data == NULL) {
        die("out of memory");
    }
    return data;
}

static int *window_buffer_new(const filter_job *job)
{
    return alloc_ints(filter_window_buffer_size(job->window));
}

static void run_start(filter_job *job)
{
    memcpy(job->a, job->src, job->rows * job->cols * sizeof(*job->a));

    pthread_mutex_lock(&job->mutex);
    job->next_block = 0;
    pthread_mutex_unlock(&job->mutex);
}

static int block_take(filter_job *job, size_t iter, size_t *block)
{
    int taken = 0;
    size_t base = iter * job->total_blocks;
    size_t limit = base + job->total_blocks;

    pthread_mutex_lock(&job->mutex);
    if (job->next_block < limit) {
        *block = job->next_block++ - base;
        taken = 1;
    }
    pthread_mutex_unlock(&job->mutex);
    return taken ? 0 : -1;
}

static void run_blocks(filter_job *job, size_t iter, const int *in, int *out, int *window_buffer)
{
    size_t block = 0;

    while (block_take(job, iter, &block) == 0) {
        size_t from = block * job->block_rows;
        size_t to = from + job->block_rows;
        if (to > job->rows) {
            to = job->rows;
        }

        median_filter_block(in, out, job->rows, job->cols, from, to,
                            job->window, window_buffer);
    }
}

static const int *run_iterations(filter_job *job, int *window_buffer)
{
    int *in = job->a;
    int *out = job->b;

    for (size_t iter = 0; iter < job->iters; ++iter) {
        if (iter > 0) {
            int *tmp = in;
            in = out;
            out = tmp;
        }
        run_blocks(job, iter, in, out, window_buffer);

        pthread_barrier_wait(&job->barrier);
    }
    return out;
}

static void *worker_main(void *arg)
{
    filter_job *job = (filter_job *)arg;
    int *window_buffer = window_buffer_new(job);

    run_iterations(job, window_buffer);
    free(window_buffer);
    return NULL;
}

int filter_job_init(filter_job *job, int *src, int *dst, size_t rows, size_t cols,
                    size_t window, size_t iters, size_t block_rows)
{
    size_t cells = rows * cols;

    memset(job, 0, sizeof(*job));
    if (pthread_mutex_init(&job->mutex, NULL) != 0) {
        fprintf(stderr, "[pool] pthread_mutex_init failed\n");
        return -1;
    }

    job->src = src;
    job->dst = dst;
    job->rows = rows;
    job->cols = cols;
    job->window = window;
    job->iters = iters;

    job->a = alloc_ints(cells);
    job->b = alloc_ints(cells);

    filter_job_set_block_rows(job, block_rows);
    return 0;
}

void filter_job_set_block_rows(filter_job *job, size_t block_rows)
{
    if (block_rows == 0 || block_rows > job->rows) {
        block_rows = job->rows;
    }
    job->block_rows = block_rows;
    job->total_blocks = (job->rows + block_rows - 1) / block_rows;
}

void filter_job_destroy(filter_job *job)
{
    pthread_mutex_destroy(&job->mutex);
    free(job->a);
    free(job->b);
    job->a = NULL;
    job->b = NULL;
}

static void save_result(filter_job *job, const int *result)
{
    memcpy(job->dst, result, job->rows * job->cols * sizeof(*job->dst));
}

int filter_run_serial(filter_job *job)
{
    int *window_buffer = window_buffer_new(job);
    int *in = job->a;
    int *out = job->b;

    run_start(job);

    double t0 = now_sec();
    for (size_t iter = 0; iter < job->iters; ++iter) {
        if (iter > 0) {
            int *tmp = in;
            in = out;
            out = tmp;
        }
        median_filter_apply(in, out, job->rows, job->cols, job->window, window_buffer);
    }
    job->elapsed = now_sec() - t0;

    save_result(job, out);
    free(window_buffer);
    return 0;
}

int filter_run(filter_job *job, size_t threads)
{
    pthread_t *tids = NULL;
    size_t workers = 0;

    if (threads == 0) {
        threads = 1;
    }
    if (threads == 1) {
        return filter_run_serial(job);
    }
    if (threads > POOL_MAX_THREADS) {
        fprintf(stderr, "[pool] too many threads: %zu (max %u)\n", threads, POOL_MAX_THREADS);
        return -1;
    }

    workers = threads - 1;
    tids = (pthread_t *)calloc(workers, sizeof(*tids));
    if (tids == NULL) {
        die("out of memory");
    }
    int *main_buffer = window_buffer_new(job);

    if (pthread_barrier_init(&job->barrier, NULL, threads) != 0) {
        die("cannot create barrier");
    }
    run_start(job);

    double t0 = now_sec();
    for (size_t i = 0; i < workers; ++i) {
        if (pthread_create(&tids[i], NULL, worker_main, job) != 0) {
            die("cannot create thread");
        }
    }

    const int *result = run_iterations(job, main_buffer);
    job->elapsed = now_sec() - t0;

    save_result(job, result);

    for (size_t i = 0; i < workers; ++i) {
        pthread_join(tids[i], NULL);
    }

    pthread_barrier_destroy(&job->barrier);

    free(main_buffer);
    free(tids);
    return 0;
}
