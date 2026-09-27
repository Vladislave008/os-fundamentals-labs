#include "os_api.h"

#include <float.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINE 1024

static int read_line(char *buf, int max)
{
    if (!fgets(buf, max, stdin))
        return 0;
    size_t n = strlen(buf);
    while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r')) {
        buf[--n] = '\0';
    }
    return 1;
}

static int is_finite_float(float x)
{
    return x == x
        && x <= FLT_MAX
        && x >= -FLT_MAX;
}

int main(void)
{
    char file_name[MAX_LINE];
    int rc = 0;

    printf("Enter file name for child: ");
    fflush(stdout);
    if (!read_line(file_name, sizeof(file_name)) || file_name[0] == '\0') {
        fprintf(stderr, "No file name provided\n");
        return 1;
    }

    os_pipe in;  /* in:  parent -> child */
    os_pipe out; /* out: child  -> parent */
    if (os_pipe_create(&in) != 0) {
        fprintf(stderr, "[parent] os_pipe_create(pipe1) failed\n");
        return 1;
    }
    if (os_pipe_create(&out) != 0) {
        fprintf(stderr, "[parent] os_pipe_create(pipe2) failed\n");
        return 1;
    }

    const char *argv[] = {"child", file_name, NULL};

    os_process proc;
    if (os_process_spawn(&proc, argv, &in, &out) != 0) {
        fprintf(stderr, "[parent] os_process_spawn failed\n");
        return 1;
    }

    fprintf(stderr, "[parent pid=%lu] child started, pid=%lu\n",
           os_self_pid(), os_process_pid(&proc));

    printf("Enter any number of float numbers per line, space-separated.\n");
    printf("Empty line or EOF to finish.\n");

    char line[MAX_LINE];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (!read_line(line, sizeof(line)))
            break;
        if (line[0] == '\0')
            break;

        const char *p = line;
        int count = 0;
        int ok = 1;
        while (ok) {
            float x;
            int n = 0;
            if (sscanf(p, "%f%n", &x, &n) != 1)
                break;
            if (n <= 0)
                break;

            if (!is_finite_float(x)) {
                fprintf(stderr, "[parent] input is not a finite float: skipped\n");
                p += n;
                continue;
            }

            if (os_pipe_write(&in, &x, sizeof(x)) != 0) {
                fprintf(stderr, "[parent] os_pipe_write failed\n");
                rc = 1;
                ok = 0;
                break;
            }

            fprintf(stderr, "[parent pid=%lu] sent %.3f\n", os_self_pid(), x);

            p += n;
            count++;
        }

        if (!ok)
            break;

        if (count == 0) {
            fprintf(stderr, "[parent] no numbers in line or bad values given, skipped\n");
        } else {
            fprintf(stderr, "[parent pid=%lu] line done: %d number(s)\n",
                    os_self_pid(), count);
        }
    }

    if (os_pipe_close_write(&in) != 0) {
        fprintf(stderr, "[parent] os_pipe_close_write(in) failed\n");
        rc = 1;
    }
    fprintf(stderr, "[parent] pipe1 closed, waiting for child's reply...\n");

    float sum;
    int r = os_pipe_read(&out, &sum, sizeof(sum));
    if (r == 1) {
        fprintf(stderr, "[parent pid=%lu] child reported sum = %.3f\n",
                os_self_pid(), sum);
    } else {
        fprintf(stderr, "[parent] failed to read sum from child\n");
        rc = 1;
    }
    if (os_pipe_close_read(&out) != 0) {
        fprintf(stderr, "[parent] os_pipe_close_read(out) failed\n");
        rc = 1;
    }

    int code = -1;
    if (os_process_wait(&proc, &code) == 0) {
        fprintf(stderr, "[parent] child exited with code %d\n", code);
    } else {
        fprintf(stderr, "[parent] os_process_wait failed\n");
        rc = 1;
    }

    if (os_process_close(&proc) != 0) {
        fprintf(stderr, "[parent] os_process_close failed\n");
        rc = 1;
    }
    return rc;
}