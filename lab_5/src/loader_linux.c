#include "loader.h"

#include <dlfcn.h>
#include <stdio.h>

LibraryHandle library_open(const char *path)
{
    LibraryHandle handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (handle == NULL) {
        fprintf(stderr, "dlopen(%s): %s\n", path, dlerror());
    }
    return handle;
}

Lab5Function library_function(LibraryHandle handle, const char *name)
{
    dlerror();
    void *address = dlsym(handle, name);
    const char *error = dlerror();
    if (error != NULL || address == NULL) {
        fprintf(stderr, "dlsym(%s): %s\n", name, error ? error : "null address");
        return NULL;
    }
    return (Lab5Function)address;
}

int library_close(LibraryHandle handle)
{
    if (dlclose(handle) != 0) {
        fprintf(stderr, "dlclose: %s\n", dlerror());
        return -1;
    }
    return 0;
}
