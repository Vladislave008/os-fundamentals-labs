#include "os_api.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void trim_line_end(char *line)
{
    size_t size = strlen(line);
    while (size > 0 && (line[size - 1] == '\n' || line[size - 1] == '\r'))
        line[--size] = '\0';
}

static int read_values(os_mapping *mapping)
{
    char *line = NULL;
    size_t capacity = 0;
    int result = 0;

    puts("Enter float numbers, separated by spaces. Empty line or EOF to finish.");
    for (;;) {
        printf("> ");
        fflush(stdout);
        if (getline(&line, &capacity, stdin) == -1) {
            if (!feof(stdin)) {
                perror("getline(numbers)");
                result = -1;
            }
            break;
        }

        trim_line_end(line);
        if (line[0] == '\0')
            break;

        size_t count_before = mapping->data->count;
        char *cursor = line;
        for (;;) {
            char *end;
            float value = strtof(cursor, &end);
            if (end == cursor)
                break;
            cursor = end;

            if (!isfinite(value)) {
                fprintf(stderr, "[parent] input is not a finite float: skipped\n");
                continue;
            }
            if (os_mapping_append(mapping, value) == -1) {
                free(line);
                return -1;
            }
        }
        if (mapping->data->count == count_before)
            fprintf(stderr, "[parent] no valid numbers in line: skipped\n");
    }
    free(line);
    return result;
}

int main(void)
{
    char *file_name = NULL;
    size_t capacity = 0;
    printf("Enter file name for child: ");
    fflush(stdout);
    if (getline(&file_name, &capacity, stdin) == -1) {
        if (feof(stdin))
            fprintf(stderr, "No file name provided\n");
        else
            perror("getline(file name)");
        free(file_name);
        return 1;
    }

    trim_line_end(file_name);
    if (file_name[0] == '\0') {
        fprintf(stderr, "No file name provided\n");
        free(file_name);
        return 1;
    }

    os_mapping mapping;
    if (os_mapping_create(&mapping) == -1) {
        free(file_name);
        return 1;
    }

    int result = 1;
    if (read_values(&mapping) == 0 && os_run_child(mapping.fd, file_name) == 0) {
        printf("Sum = %.6f\n", mapping.data->sum);
        result = 0;
    }
    if (os_mapping_close(&mapping) == -1)
        result = 1;
    free(file_name);
    return result;
}
