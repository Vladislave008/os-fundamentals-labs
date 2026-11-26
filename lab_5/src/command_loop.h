#ifndef LAB5_COMMAND_LOOP_H
#define LAB5_COMMAND_LOOP_H

#include "lab5.h"

typedef struct {
    Lab5Function prime_count;
    Lab5Function gcf;
} Lab5Functions;

typedef struct Runtime Runtime;

int run_commands(Lab5Functions *functions, int (*switch_library)(Runtime *),
                 Runtime *context);

#endif
