#ifndef LAB3_QSORT_H
#define LAB3_QSORT_H

#include <stddef.h>

size_t part(int *data, size_t lo, size_t hi);

void qsort_core(int *data, size_t lo, size_t hi);

void int_qsort(int *data, size_t size);

#endif
