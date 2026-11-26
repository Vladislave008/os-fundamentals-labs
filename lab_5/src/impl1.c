#include "lab5.h"

static int is_prime(int number)
{
    for (int divisor = 2; divisor < number; ++divisor) {
        if (number % divisor == 0) {
            return 0;
        }
    }
    return 1;
}

int PrimeCount(int A, int B)
{
    if (A <= 0 || A > B) {
        return -1;
    }

    int count = 0;
    for (int number = A; ; ++number) {
        if (number >= 2 && is_prime(number)) {
            ++count;
        }
        if (number == B) {
            return count;
        }
    }
}

int GCF(int A, int B)
{
    if (A <= 0 || B <= 0) {
        return -1;
    }

    while (B != 0) {
        int remainder = A % B;
        A = B;
        B = remainder;
    }
    return A;
}
