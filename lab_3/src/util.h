#ifndef LAB3_UTIL_H
#define LAB3_UTIL_H

#include <stddef.h>

void die(const char *msg);

size_t parse_size_range(const char *s, size_t min, size_t max);

#endif
