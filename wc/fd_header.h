#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

#ifndef DATA_WORK_HEADER
#define DETA_WORK_HEADER 

#define MAX_PATH_LEN 128

int safe_open(const char* pathname, int flags, int mode);

void safe_close(int fd);

ssize_t safe_read(int fd, char* buf, size_t buf_sz);

ssize_t safe_write(int fd, const char* buf, size_t size);

#endif
