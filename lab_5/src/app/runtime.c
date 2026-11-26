#include "lab5.h"
#include "command_loop.h"
#include "loader.h"

#include <stdio.h>
#include <stdlib.h>

struct Runtime {
    LibraryHandle handle;
    Lab5Functions functions;
    int implementation;
};

static int load_implementation(Runtime *runtime, int implementation)
{
    const char *path = implementation == 1 ? IMPL1_PATH : IMPL2_PATH;
    LibraryHandle handle = library_open(path);
    if (handle == NULL) {
        return 1;
    }

    Lab5Functions functions = {
        library_function(handle, "PrimeCount"),
        library_function(handle, "GCF")
    };
    if (functions.prime_count == NULL || functions.gcf == NULL) {
        return library_close(handle) == 0 ? 1 : -1;
    }

    if (runtime->handle != NULL && library_close(runtime->handle) != 0) {
        library_close(handle);
        return -1;
    }

    *runtime = (Runtime){ handle, functions, implementation };
    return 0;
}

static int switch_implementation(Runtime *runtime)
{
    int next = runtime->implementation == 1 ? 2 : 1;
    int status = load_implementation(runtime, next);
    if (status == 0) {
        printf("Implementation %d active.\n", next);
    } else if (status > 0) {
        fprintf(stderr, "Implementation %d remains active.\n", runtime->implementation);
    }
    return status < 0 ? -1 : 0;
}

int main(void)
{
    Runtime runtime = {0};
    if (load_implementation(&runtime, 1) != 0) {
        return EXIT_FAILURE;
    }

    int status = run_commands(&runtime.functions, switch_implementation, &runtime);
    if (library_close(runtime.handle) != 0) {
        return EXIT_FAILURE;
    }
    return status;
}
