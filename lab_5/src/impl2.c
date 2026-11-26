#include "lab5.h"

#include <stdlib.h>

int PrimeCount(int A, int B)
{
    if (A <= 0 || A > B) {
        return -1;
    }

    size_t limit = (size_t)B;
    unsigned char *composite = calloc(limit + 1, sizeof *composite);
    if (composite == NULL) {
        return -1;
    }

    for (size_t prime = 2; prime <= limit / prime; ++prime) {
        if (composite[prime]) {
            continue;
        }
        for (size_t multiple = prime * prime; multiple <= limit; multiple += prime) {
            composite[multiple] = 1;
        }
    }

    int count = 0;
    for (size_t number = A < 2 ? 2 : (size_t)A; number <= limit; ++number) {
        if (!composite[number]) {
            ++count;
        }
    }
    free(composite);
    return count;
}

int GCF(int A, int B)
{
    if (A <= 0 || B <= 0) {
        return -1;
    }

    int divisor = A < B ? A : B;
    while (A % divisor != 0 || B % divisor != 0) {
        --divisor;
    }
    return divisor;
}
