#ifndef LAB3_MATRIX_FILTER_H
#define LAB3_MATRIX_FILTER_H

#include <stddef.h>

int median(int* data, size_t size);

void median_filter_pixel(const int* in_matrix, int* out_matrix, size_t rows, size_t cols, 
                        size_t pixel_row, size_t pixel_col, 
                        size_t window_width,
                        int* window_buffer);

void median_filter_block(const int* in_matrix, int* out_matrix, size_t rows, size_t cols,
                        size_t first_row, size_t last_row,
                        size_t window_width,
                        int* window_buffer);

void median_filter_apply(const int* in_matrix, int* out_matrix, size_t rows, size_t cols,
                        size_t window_width,
                        int* window_buffer);

size_t filter_window_buffer_size(size_t window_width);

#endif
