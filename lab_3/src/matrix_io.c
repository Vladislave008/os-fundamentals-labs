#include <stdio.h>
#include <stdlib.h>

#include "matrix_io.h"
#include "util.h"

static int *matrix_alloc(matrix *m, size_t rows, size_t cols)
{
    int *data = (int *)malloc(rows * cols * sizeof(*data));

    if (data == NULL) {
        die("out of memory");
    }
    m->data = data;
    m->rows = rows;
    m->cols = cols;
    return data;
}

void matrix_load_file(const char *path, matrix *m, size_t rows, size_t cols)
{
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        die("cannot open file");
    }

    size_t n = rows * cols;
    int *data = matrix_alloc(m, rows, cols);
    for (size_t i = 0; i < n; ++i) {
        if (fscanf(f, "%d", &data[i]) != 1) {
            die("unexpected end of file");
        }
    }

    fclose(f);
}

void matrix_load_stdin(matrix *m, size_t rows, size_t cols)
{
    if (rows == 0) {
        printf("Enter rows: ");
        fflush(stdout);
        if (scanf("%zu", &rows) != 1) {
            die("invalid rows\n");
        }
    }
    if (cols == 0) {
        printf("Enter cols: ");
        fflush(stdout);
        if (scanf("%zu", &cols) != 1) {
            die("invalid cols\n");
        }
    }

    size_t n = rows * cols;
    int *data = matrix_alloc(m, rows, cols);

    printf("Enter %zu integers:\n> ", n);
    fflush(stdout);
    for (size_t i = 0; i < n; ++i) {
        if (scanf("%d", &data[i]) != 1) {
            die("invalid integer");
        }
    }
}

void matrix_generate(matrix *m, size_t rows, size_t cols, unsigned long seed, int min_val, int max_val)
{
    if (min_val > max_val) {
        die("min value must not exceed max");
    }

    int *data = matrix_alloc(m, rows, cols);
    int span = max_val - min_val + 1;

    srand((unsigned int)seed);
    for (size_t i = 0, n = rows * cols; i < n; ++i) {
        data[i] = min_val + rand() % span;
    }
}

void matrix_save(const char *path, const matrix *m)
{
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        die("cannot create file");
    }

    for (size_t r = 0; r < m->rows; ++r) {
        for (size_t c = 0; c < m->cols; ++c) {
            int rc = fprintf(f, "%d%c", m->data[r * m->cols + c], c + 1 < m->cols ? ' ' : '\n');
            if (rc < 0) {
                die("write error");
            }
        }
    }

    if (fclose(f) != 0) {
        die("cannot close file");
    }
}
