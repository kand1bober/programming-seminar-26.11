#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>

int safe_open(const char* pathname, int flags, int mode)
{   
    int fd = open(pathname, flags, mode); //rights assigning only when creating 

    if (fd < 0)
    {
        perror("error in open syscall");
        exit(EXIT_FAILURE);
    }

    return fd;
}


void safe_close(int fd)
{
    if (close(fd) < 0)
    {  
        perror("error in close syscall");
        exit(EXIT_FAILURE);
    }
}

ssize_t safe_read(int fd, char* buf, size_t buf_sz)
{
    ssize_t n = 0, read_bytes = 0;
    while (1)
    {   
        n = read(fd, buf + read_bytes, buf_sz - read_bytes);

        if (n < 0)
        {
            if (errno == EINTR)
            {
                n = 0;
                continue;
            }

            perror("error in read syscall");
            exit(EXIT_FAILURE);
        }
        if (n == 0)
            break;

        read_bytes += n;
    }

    return read_bytes;
}


ssize_t safe_write(int fd, const char* buf, size_t size)
{
    ssize_t n = 0 ,written = 0;
    while (1)
    {
        n = write(fd, buf + written, size - written);

        if (n < 0)
        {
            if (errno == EINTR) 
            {
                n = 0;
                continue;
            }

            perror("error in write syscall");
            exit(EXIT_FAILURE);
        }
        if (n == 0)
            break;

        written += n;
    }

    return written;
}
