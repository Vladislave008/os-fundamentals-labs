#ifndef LAB2_OS_API_H
#define LAB2_OS_API_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uintptr_t read; // int or HANDLE
    uintptr_t write;
} os_pipe;

typedef struct {
    uintptr_t proc; // PID or HANDLE
    unsigned long pid; // for logs
} os_process;

unsigned long os_self_pid(void); // PID of current process

int os_pipe_create(os_pipe *p); // 0 - success; -1 - error (reason to stderr)

int os_pipe_close_read(os_pipe *p);  // 0 - success; -1 - error (reason to stderr)
int os_pipe_close_write(os_pipe *p); // 0 - success; -1 - error (reason to stderr)

int os_pipe_read(os_pipe *p, void *buf, size_t size); // 1 - read size bytes to buf; 0 - got EOF; -1 - error
int os_pipe_write(os_pipe *p, const void *buf, size_t size); // 0 - wrote size bytes from buf; -1 - error

int os_process_spawn(os_process *out, const char *const argv[],
                     os_pipe *in, os_pipe *out_pipe);
/* 
    argv[0] - executable file name,
    argv[1...n-1] - args for executable
    argv[n] - NULL
    0 = success, -1 = error. 
*/

unsigned long os_process_pid(const os_process *p);

int os_process_wait(os_process *p, int *exit_code);
/*  
    wait until process ends; 
    0 - success, -1 - error
*/

int os_process_close(os_process *p); // clear finished process resources; 0 - success, -1 - error

#endif