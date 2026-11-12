#ifndef LAB4_OS_API_H
#define LAB4_OS_API_H

#include <stddef.h>

typedef struct {
    size_t count;
    float sum;
    float values[];
} shared_data;

typedef struct {
    int fd;
    size_t size;
    shared_data *data;
} os_mapping;

int os_mapping_create(os_mapping *mapping);
int os_mapping_open(os_mapping *mapping, int fd);
int os_mapping_append(os_mapping *mapping, float value);
int os_mapping_close(os_mapping *mapping);
int os_run_child(int fd, const char *file_name);

#endif
