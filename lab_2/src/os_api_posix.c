#include "os_api.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define OS_INVALID ((uintptr_t)-1) // value for closed handle, UINTPTR_MAX

static void report(const char *msg)
{
    fprintf(stderr, "[os_api:%s] errno %d: %s\n", msg, errno, strerror(errno));
}

unsigned long os_self_pid(void)
{
    return (unsigned long)getpid();
}

int os_pipe_create(os_pipe *p)
{
    int fds[2];
    if (pipe(fds) < 0)
    {
        report("pipe");
        return -1;
    }
    p->read = (uintptr_t)fds[0];
    p->write = (uintptr_t)fds[1];
    return 0;
}

int os_pipe_close_read(os_pipe *p)
{
    if (p->read != OS_INVALID)
    {
        if (close((int)p->read) < 0)
        {
            report("close(read)");
            return -1;
        }
        p->read = OS_INVALID;
    }
    return 0;
}

int os_pipe_close_write(os_pipe *p)
{
    if (p->write != OS_INVALID)
    {
        if (close((int)p->write) < 0)
        {
            report("close(write)");
            return -1;
        }
        p->write = OS_INVALID;
    }
    return 0;
}

int os_pipe_read(os_pipe *p, void *buf, size_t size)
{
    ssize_t rd = read((int)p->read, buf, size);
    if (rd < 0)
    {
        report("read(pipe)");
        return -1;
    }
    return rd == (ssize_t)size ? 1 : 0;
}

int os_pipe_write(os_pipe *p, const void *buf, size_t size)
{
    ssize_t wr = write((int)p->write, buf, size);
    if (wr != (ssize_t)size)
    {
        if (wr < 0)
            report("write(pipe)");
        else
            fprintf(stderr, "[os_api] short write\n");
        return -1;
    }
    return 0;
}

int os_process_spawn(os_process *out, const char *const argv[],
                     os_pipe *in, os_pipe *out_pipe)
{
    int in_r = (int)in->read;
    int in_w = (int)in->write;
    int out_r = (int)out_pipe->read;
    int out_w = (int)out_pipe->write;

    pid_t pid = fork();
    if (pid < 0)
    {
        report("fork");
        return -1;
    }

    if (pid == 0)
    { // child process
        if (dup2(in_r, STDIN_FILENO) < 0)
        {
            fprintf(stderr, "[os_api] child: dup2(in_r, STDIN) failed\n");
            _exit(127);
        }
        if (dup2(out_w, STDOUT_FILENO) < 0)
        {
            fprintf(stderr, "[os_api] child: dup2(out_w, STDOUT) failed\n");
            _exit(127);
        }
        if (os_pipe_close_read(in) != 0)
        {
            fprintf(stderr, "[os_api] child: close pipe end failed\n");
            _exit(127);
        }
        if (os_pipe_close_write(in) != 0)
        {
            fprintf(stderr, "[os_api] child: close pipe end failed\n");
            _exit(127);
        }
        if (os_pipe_close_read(out_pipe) != 0)
        {
            fprintf(stderr, "[os_api] child: close pipe end failed\n");
            _exit(127);
        }
        if (os_pipe_close_write(out_pipe) != 0)
        {
            fprintf(stderr, "[os_api] child: close pipe end failed\n");
            _exit(127);
        }
        const char *prog = argv[0];
        if (strchr(prog, '/'))
        {
            execv(prog, (char *const *)argv);
        }
        else
        {
            char path[4096];
            size_t len = strlen(prog);
            if (len >= sizeof(path) - 2)
            {
                fprintf(stderr, "[os_api] child: program name too long\n");
                _exit(127);
            }
            path[0] = '.';
            path[1] = '/';
            memcpy(path + 2, prog, len + 1);
            execv(path, (char *const *)argv);
        }
        fprintf(stderr, "[os_api] child: execv(%s) failed\n", prog);
        _exit(127);
    }

    // main process
    out->proc = (uintptr_t)pid;
    out->pid = (unsigned long)pid;

    int rc = 0;
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
    int status = 0;
    if (waitpid((pid_t)p->proc, &status, 0) < 0)
    {
        report("waitpid");
        return -1;
    }
    if (WIFEXITED(status))
    {
        *exit_code = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        *exit_code = 128 + WTERMSIG(status);
    }
    else
    {
        *exit_code = -1;
    }
    p->proc = OS_INVALID;
    return 0;
}

int os_process_close(os_process *p)
{
    p->proc = OS_INVALID;
    return 0;
}