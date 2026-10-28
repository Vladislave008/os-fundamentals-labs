#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

void die(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

static int all_digits(const char *s, size_t from)
{
    if (s[from] == '\0') {
        return 0;
    }
    for (const char *p = s + from; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return 0;
        }
    }
    return 1;
}

static int parse_long(const char *s, long *out)
{
    char *end = NULL;

    if (s == NULL || *s == '\0') {
        return -1;
    }
    if ((*s == '+' || *s == '-') && !all_digits(s, 1)) {
        return -1;
    }

    errno = 0;
    long value = strtol(s, &end, 10);
    if (errno == ERANGE || end == s || end == NULL || *end != '\0') {
        return -1;
    }
    *out = value;
    return 0;
}

size_t parse_size_range(const char *s, size_t min, size_t max)
{
    long value = 0;

    if (parse_long(s, &value) != 0) {
        die("argument is not an integer");
    }
    if (value < 0 || (unsigned long)value > (unsigned long)SIZE_MAX) {
        die("argument out of range");
    }
    if ((size_t)value < min || (size_t)value > max) {
        die("argument out of range");
    }
    return (size_t)value;
}

