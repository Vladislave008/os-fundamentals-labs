#include "os_api.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static int report_error(const char *operation)
{
    perror(operation);
    return -1;
}

static int map_file(os_mapping *mapping, int fd, size_t size)
{
    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
        return report_error("mmap");

    *mapping = (os_mapping){fd, size, data};
    return 0;
}

static int resize_file(int fd, size_t size)
{
    if (ftruncate(fd, (off_t)size) == -1)
        return report_error("ftruncate");
    return 0;
}

int os_mapping_create(os_mapping *mapping)
{
    char path[] = "/tmp/os_lab4_XXXXXX";
    int fd = mkstemp(path);
    if (fd == -1)
        return report_error("mkstemp");
    if (unlink(path) == -1) {
        perror("unlink");
    } else {
        size_t size = sizeof(shared_data) + 64 * sizeof(float);
        if (resize_file(fd, size) == 0 && map_file(mapping, fd, size) == 0) {
            mapping->data->count = 0;
            return 0;
        }
    }

    if (close(fd) == -1)
        perror("close");
    return -1;
}

int os_mapping_open(os_mapping *mapping, int fd)
{
    struct stat info;
    if (fstat(fd, &info) == -1)
        return report_error("fstat");
    return map_file(mapping, fd, (size_t)info.st_size);
}

int os_mapping_append(os_mapping *mapping, float value)
{
    size_t count = mapping->data->count;
    size_t capacity = (mapping->size - sizeof(shared_data)) / sizeof(float);
    if (count == capacity) {
        if (mapping->size > SIZE_MAX / 2) {
            errno = EFBIG;
            return report_error("mapping size");
        }
        size_t size = mapping->size * 2;
        if (resize_file(mapping->fd, size) == -1)
            return -1;
        void *data = mremap(mapping->data, mapping->size, size, MREMAP_MAYMOVE);
        if (data == MAP_FAILED)
            return report_error("mremap");
        mapping->data = data;
        mapping->size = size;
    }
    mapping->data->values[count] = value;
    mapping->data->count = count + 1;
    return 0;
}

int os_mapping_close(os_mapping *mapping)
{
    int result = 0;
    if (munmap(mapping->data, mapping->size) == -1)
        result = report_error("munmap");
    if (close(mapping->fd) == -1)
        result = report_error("close");
    return result;
}

int os_run_child(int fd, const char *file_name)
{
    char descriptor[32];
    snprintf(descriptor, sizeof(descriptor), "%d", fd);
    pid_t pid = fork();
    if (pid == -1)
        return report_error("fork");
    if (pid == 0) {
        execl("./child", "child", descriptor, file_name, (char *)NULL);
        perror("execl(child)");
        _exit(127);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1)
        return report_error("waitpid");
    if (WIFSIGNALED(status)) {
        fprintf(stderr, "[parent] child terminated by signal %d\n", WTERMSIG(status));
        return -1;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "[parent] child failed\n");
        return -1;
    }
    return 0;
}
