#include <stddef.h>

#include "qsort.h"

size_t part(int* data, size_t lo, size_t hi) {
    int pivot = data[lo + (hi - lo) / 2];
    int i = lo-1;
    int j = hi+1;
    while (1) {
        do {j--;} while (data[j] > pivot);
        do {i++;} while (data[i] < pivot); 
        if (i >= j) {
            return j;
        }
        int temp = data[j];
        data[j] = data[i];
        data[i] = temp;
    }
}

void qsort_core(int* data, size_t lo, size_t hi) {
    if (lo < hi) {
        size_t p = part(data, lo, hi);
        qsort_core(data, lo, p);
        qsort_core(data, p+1, hi);
    }

}

void int_qsort(int* data, size_t size) {
    if (size < 2) {
        return;
    }
    qsort_core(data, 0, size-1);
}
