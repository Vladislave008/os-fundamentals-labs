#include <stddef.h>

#include "matrix_filter.h"
#include "qsort.h"

int median(int* data, size_t size) {
    int_qsort(data, size);
    return data[size/2];
}

void median_filter_pixel(const int* in_matrix, int* out_matrix, size_t rows, size_t cols, 
                        size_t pixel_row, size_t pixel_col, 
                        size_t window_width,
                        int* window_buffer) {
    int half = (int)(window_width / 2);
    size_t cnt = 0;
    for (int i = -half; i <= half; ++i) {
        for (int j = -half; j <= half; ++j) {
            int r = (int)pixel_row + i;
            int c = (int)pixel_col + j;
            if (r >= 0 && c >= 0 && r < (int)rows && c < (int)cols) {
                window_buffer[cnt++] = in_matrix[(size_t)r*cols+(size_t)c];
            }
        }
    }
    out_matrix[pixel_row*cols+pixel_col] = median(window_buffer, cnt);
}

void median_filter_block(const int* in_matrix, int* out_matrix, size_t rows, size_t cols,
                        size_t first_row, size_t last_row,
                        size_t window_width,
                        int* window_buffer) {
    if (first_row >= last_row) {
        return;
    }
    for (size_t r = first_row; r < last_row; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            median_filter_pixel(in_matrix, out_matrix, rows, cols, r, c, window_width, window_buffer);
        }
    }
}

void median_filter_apply(const int* in_matrix, int* out_matrix, size_t rows, size_t cols,
                        size_t window_width,
                        int* window_buffer) {
    median_filter_block(in_matrix, out_matrix, rows, cols, 0, rows, window_width, window_buffer);
}

size_t filter_window_buffer_size(size_t window_width) {
    return window_width * window_width;
}
