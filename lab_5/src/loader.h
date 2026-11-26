#ifndef LAB5_LOADER_H
#define LAB5_LOADER_H

#include "lab5.h"

typedef void *LibraryHandle;

LibraryHandle library_open(const char *path);
Lab5Function library_function(LibraryHandle handle, const char *name);
int library_close(LibraryHandle handle);

#endif
