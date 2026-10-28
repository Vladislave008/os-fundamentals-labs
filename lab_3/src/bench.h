#ifndef LAB3_BENCH_H
#define LAB3_BENCH_H

#include <stddef.h>

double now_sec(void);

void bench_stats(const double *data, size_t n, double *best, double *median);

#endif
