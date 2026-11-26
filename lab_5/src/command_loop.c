#include "command_loop.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_int(const char **cursor, int *value)
{
    char *end;
    errno = 0;
    long number = strtol(*cursor, &end, 10);
    if (end == *cursor || errno == ERANGE || number < INT_MIN || number > INT_MAX
        || (*end != '\0' && !isspace((unsigned char)*end))) {
        return 0;
    }
    *cursor = end;
    *value = (int)number;
    return 1;
}

static int end_of_line(const char *cursor)
{
    while (isspace((unsigned char)*cursor)) {
        ++cursor;
    }
    return *cursor == '\0';
}

int run_commands(Lab5Functions *functions, int (*switch_library)(Runtime *),
                 Runtime *context)
{
    char line[256];
    while (fgets(line, sizeof line, stdin) != NULL) {
        if (strchr(line, '\n') == NULL && !feof(stdin)) {
            fputs("Input line is too long.\n", stderr);
            return EXIT_FAILURE;
        }

        const char *cursor = line;
        if (end_of_line(cursor)) {
            continue;
        }
        int command, A, B;
        if (!read_int(&cursor, &command)) {
            command = -1;
        }
        if (command == 0 && end_of_line(cursor)) {
            if (switch_library == NULL) {
                fputs("Switching is available only in program2.\n", stderr);
            } else if (switch_library(context) != 0) {
                return EXIT_FAILURE;
            }
        } else if ((command == 1 || command == 2)
                   && read_int(&cursor, &A) && read_int(&cursor, &B)
                   && end_of_line(cursor) && A > 0 && B > 0
                   && (command == 2 || A <= B)) {
            Lab5Function function = command == 1 ? functions->prime_count : functions->gcf;
            int result = function(A, B);
            if (result < 0) {
                fprintf(stderr, "Calculation failed.\n");
            } else {
                printf("%d\n", result);
            }
        } else {
            fprintf(stderr, "Expected 1 A B (0 < A <= B), 2 A B (A, B > 0), or 0.\n");
        }
    }

    if (ferror(stdin)) {
        fprintf(stderr, "stdin: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
