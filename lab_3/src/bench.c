#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#include "bench.h"

double now_sec(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0.0;
    }
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static int cmp_double(const void *a, const void *b)
{
    double x = *(const double *)a;
    double y = *(const double *)b;

    return (x > y) - (x < y);
}

void bench_stats(const double *data, size_t n, double *best, double *median)
{
    if (n == 0) {
        *best = 0.0;
        *median = 0.0;
        return;
    }

    double local_min = data[0];
    for (size_t i = 1; i < n; ++i) {
        if (data[i] < local_min) {
            local_min = data[i];
        }
    }
    *best = local_min;

    double *sorted = (double *)malloc(n * sizeof(*sorted));
    if (sorted == NULL) {
        *median = *best;
        return;
    }
    for (size_t i = 0; i < n; ++i) {
        sorted[i] = data[i];
    }
    qsort(sorted, n, sizeof(*sorted), cmp_double);
    *median = (n % 2 == 1) ? sorted[n / 2] : 0.5 * (sorted[n / 2 - 1] + sorted[n / 2]);
    free(sorted);
}
