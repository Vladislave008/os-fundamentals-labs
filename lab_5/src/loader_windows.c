#include "loader.h"

#include <stdio.h>
#include <windows.h>

LibraryHandle library_open(const char *path)
{
    wchar_t relative_path[MAX_PATH];
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1,
                            relative_path, MAX_PATH)) {
        fprintf(stderr, "Invalid library path %s: Windows error %lu\n",
                path, (unsigned long)GetLastError());
        return NULL;
    }
    wchar_t full_path[32768];
    DWORD capacity = 32768;
    DWORD length = GetFullPathNameW(relative_path, capacity, full_path, NULL);
    if (length == 0) {
        fprintf(stderr, "GetFullPathName(%s): Windows error %lu\n",
                path, (unsigned long)GetLastError());
        return NULL;
    }
    if (length >= capacity) {
        fprintf(stderr, "Library path is too long: %s\n", path);
        return NULL;
    }

    HMODULE handle = LoadLibraryW(full_path);
    if (handle == NULL) {
        fprintf(stderr, "LoadLibrary(%s): Windows error %lu\n",
                path, (unsigned long)GetLastError());
    }
    return handle;
}

Lab5Function library_function(LibraryHandle handle, const char *name)
{
    FARPROC address = GetProcAddress((HMODULE)handle, name);
    if (address == NULL) {
        fprintf(stderr, "GetProcAddress(%s): Windows error %lu\n",
                name, (unsigned long)GetLastError());
        return NULL;
    }
    return (Lab5Function)address;
}

int library_close(LibraryHandle handle)
{
    if (!FreeLibrary((HMODULE)handle)) {
        fprintf(stderr, "FreeLibrary: Windows error %lu\n",
                (unsigned long)GetLastError());
        return -1;
    }
    return 0;
}
