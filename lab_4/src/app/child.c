#include "os_api.h"

#include <stdio.h>
#include <stdlib.h>

static int write_result(const char *file_name, size_t count, float sum)
{
    FILE *file = fopen(file_name, "w");
    if (file == NULL) {
        perror("fopen(output file)");
        return 1;
    }

    int result = 0;
    if (fprintf(file, "Sum of %zu numbers = %.6f\n", count, sum) < 0) {
        perror("fprintf(output file)");
        result = 1;
    }
    if (fclose(file) == EOF) {
        perror("fclose(output file)");
        result = 1;
    }
    return result;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "usage: child <shared_fd> <file_name>\n");
        return 1;
    }

    os_mapping mapping;
    if (os_mapping_open(&mapping, atoi(argv[1])) == -1)
        return 1;

    size_t count = mapping.data->count;
    float sum = 0.0f;
    for (size_t i = 0; i < count; ++i)
        sum += mapping.data->values[i];

    int result = write_result(argv[2], count, sum);
    if (result == 0)
        mapping.data->sum = sum;
    if (os_mapping_close(&mapping) == -1)
        result = 1;
    return result;
}
