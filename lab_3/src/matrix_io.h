#ifndef LAB3_MATRIX_IO_H
#define LAB3_MATRIX_IO_H

#include <stddef.h>

#define MATRIX_MAX_DIM 100000u

typedef struct {
    int *data;
    size_t rows;
    size_t cols;
} matrix;

void matrix_load_file(const char *path, matrix *m, size_t rows, size_t cols);

void matrix_load_stdin(matrix *m, size_t rows, size_t cols);

void matrix_generate(matrix *m, size_t rows, size_t cols, unsigned long seed, int min_val, int max_val);

void matrix_save(const char *path, const matrix *m);

#endif
