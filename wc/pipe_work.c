#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "pipe_header.h"

void open_pipe(int* pipefd)
{
    if(pipe(pipefd) == -1)
    {
        perror("error in opening pipe");
        exit(EXIT_FAILURE);
    }
}
