#include "os_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define OS_INVALID ((uintptr_t)-1) // == INVALID_HANDLE_VALUE

static void report(const char *msg)
{
    DWORD err = GetLastError(); // DWORD - uint32_t
    fprintf(stderr, "[os_api:%s] Win32 error %lu: ", msg, (unsigned long)err);

    LPSTR buf = NULL;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&buf, 0, NULL);
    if (buf) {
        fprintf(stderr, "%s", buf);
        LocalFree(buf);
    } else {
        fprintf(stderr, "(no description)\n");
    }
}

unsigned long os_self_pid(void)
{
    return (unsigned long)GetCurrentProcessId();
}

int os_pipe_create(os_pipe *p)
{
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL; // default
    sa.bInheritHandle = TRUE;

    HANDLE r = NULL, w = NULL;
    if (!CreatePipe(&r, &w, &sa, 0)) {
        report("CreatePipe");
        return -1;
    }
    p->read = (uintptr_t)r;
    p->write = (uintptr_t)w;
    return 0;
}

int os_pipe_close_read(os_pipe *p)
{
    if (p->read != OS_INVALID) {
        if (!CloseHandle((HANDLE)p->read)) {
            report("CloseHandle(read)");
            return -1;
        }
        p->read = OS_INVALID;
    }
    return 0;
}

int os_pipe_close_write(os_pipe *p)
{
    if (p->write != OS_INVALID) {
        if (!CloseHandle((HANDLE)p->write)) {
            report("CloseHandle(write)");
            return -1;
        }
        p->write = OS_INVALID;
    }
    return 0;
}

static int set_inherit(HANDLE h, BOOL on)
{
    return SetHandleInformation(h, HANDLE_FLAG_INHERIT,
                                on ? HANDLE_FLAG_INHERIT : 0);
}

static int has_extension(const char *name)
{
    const char *base = name;
    for (const char *p = name; *p; ++p) {
        if (*p == '/' || *p == '\\')
            base = p + 1;
    }
    const char *dot = strrchr(base, '.');
    return dot != NULL && dot != base && dot[1] != '\0';
}

static int build_cmdline(const char *const argv[], char *buf, size_t size)
{
    size_t used = 0;
    for (int i = 0; argv[i] != NULL; ++i) {
        const char *s = argv[i];
        size_t len = strlen(s);
        size_t need = 2 /* кавычки */ + len + (i == 0 && !has_extension(s) ? 4 /* ".exe" */ : 0);
        if (i > 0)
            need += 1; /* пробел */
        if (used + need + 1 > size)
            return -1;

        if (i > 0)
            buf[used++] = ' ';
        buf[used++] = '"';
        memcpy(buf + used, s, len);
        used += len;
        if (i == 0 && !has_extension(s)) {
            memcpy(buf + used, ".exe", 4);
            used += 4;
        }
        buf[used++] = '"';
    }
    buf[used] = '\0';
    return 0;
}

int os_pipe_read(os_pipe *p, void *buf, size_t size)
{
    DWORD rd = 0;
    if (!ReadFile((HANDLE)p->read, buf, (DWORD)size, &rd, NULL)) {
        report("ReadFile");
        return -1;
    }
    return rd == size ? 1 : 0;
}

int os_pipe_write(os_pipe *p, const void *buf, size_t size)
{
    DWORD written = 0;
    if (!WriteFile((HANDLE)p->write, buf, (DWORD)size, &written, NULL)) {
        report("WriteFile");
        return -1;
    }
    if (written != (DWORD)size) {
        fprintf(stderr, "[os_api] short write\n");
        return -1;
    }
    return 0;
}

int os_process_spawn(os_process *out, const char *const argv[],
                     os_pipe *in, os_pipe *out_pipe)
{
    if (!set_inherit((HANDLE)in->read, TRUE)) {
        report("SetHandleInformation(in.read)");
        return -1;
    }
    if (!set_inherit((HANDLE)out_pipe->write, TRUE)) {
        report("SetHandleInformation(out.write)");
        return -1;
    }
    if (!set_inherit((HANDLE)in->write, FALSE)) {
        report("SetHandleInformation(in.write)");
        return -1;
    }
    if (!set_inherit((HANDLE)out_pipe->read, FALSE)) {
        report("SetHandleInformation(out.read)");
        return -1;
    }

    char *cmd = (char *)malloc(32768);
    if (!cmd) {
        fprintf(stderr, "[os_api] out of memory\n");
        return -1;
    }
    if (build_cmdline(argv, cmd, 32768) != 0) {
        fprintf(stderr, "[os_api] command line too long\n");
        free(cmd);
        return -1;
    }

    WCHAR wcmd[32768];
    if (MultiByteToWideChar(CP_ACP, 0, cmd, -1, wcmd, 32768) == 0) {
        free(cmd);
        report("MultiByteToWideChar");
        return -1;
    }
    free(cmd);

    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = (HANDLE)in->read;
    si.hStdOutput = (HANDLE)out_pipe->write;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    if (!CreateProcessW(NULL, wcmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        report("CreateProcessW");
        return -1;
    }

    out->proc = (uintptr_t)pi.hProcess;
    out->pid = (unsigned long)pi.dwProcessId;

    int rc = 0;
    if (!CloseHandle(pi.hThread)) {
        report("CloseHandle(thread)");
        rc = -1;
    }
    if (os_pipe_close_read(in) != 0)
        rc = -1;
    if (os_pipe_close_write(out_pipe) != 0)
        rc = -1;
    return rc;
}

unsigned long os_process_pid(const os_process *p)
{
    return p->pid;
}

int os_process_wait(os_process *p, int *exit_code)
{
    WaitForSingleObject((HANDLE)p->proc, INFINITE);
    DWORD code = 0;
    if (!GetExitCodeProcess((HANDLE)p->proc, &code)) {
        report("GetExitCodeProcess");
        return -1;
    }
    *exit_code = (int)code;
    if (!CloseHandle((HANDLE)p->proc)) {
        report("CloseHandle(process)");
        return -1;
    }
    p->proc = OS_INVALID;
    return 0;
}

int os_process_close(os_process *p)
{
    if (p->proc != OS_INVALID) {
        if (!CloseHandle((HANDLE)p->proc)) {
            report("CloseHandle(process)");
            return -1;
        }
        p->proc = OS_INVALID;
    }
    return 0;
}