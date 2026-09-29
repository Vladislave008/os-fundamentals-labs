#include "os_api.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: child <file_name>\n");
        return 1;
    }
    const char *file_name = argv[1];

    fprintf(stderr, "[child pid=%lu] started, file='%s'\n",
            os_self_pid(), file_name);

    FILE *fout = fopen(file_name, "w");
    if (!fout) {
        perror("fopen(output file)");
        return 1;
    }

    float x;
    float sum = 0.0f;
    int count = 0;

    while (fread(&x, sizeof(float), 1, stdin) == 1) {
        sum += x;
        count++;
        fprintf(stderr, "[child pid=%lu] got %.3f, running sum = %.3f\n",
                os_self_pid(), x, sum);
    }

    fprintf(stderr, "[child pid=%lu] EOF on stdin, %d number(s) total\n",
            os_self_pid(), count);

    if (fprintf(fout, "Sum of %d numbers = %.6f\n", count, sum) < 0) {
        perror("fprintf(output file)");
        fclose(fout);
        return 1;
    }
    fclose(fout);
    fprintf(stderr, "[child pid=%lu] result written to '%s'\n",
            os_self_pid(), file_name);

    if (fwrite(&sum, sizeof(sum), 1, stdout) != 1) {
        perror("fwrite(stdout)");
        return 1;
    }
    fflush(stdout);

    fprintf(stderr, "[child pid=%lu] exiting\n", os_self_pid());
    return 0;
}