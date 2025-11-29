#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/epoll.h>

#include "fd_header.h"
#include "pipe_header.h"

void process_buf(char* buf, ssize_t size, ssize_t* ans);
int process_pipe(int fd_pipe_outm, ssize_t* ans);
void child(int argc, char* argv[], int pipe_arr[][2]);

extern int errno;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("too few arguments");
        exit(0);
    }

    //open pipe
    int pipe_arr[2][2]; //0 = stdin, 1 = stderr 

    for (int i = 0; i < 2; i++) {
        open_pipe(pipe_arr[i]);
    }

    ssize_t ans[2][3] = {0};

    pid_t pid = fork();
    if (pid < 0) {
        perror("error in forking");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) { //child
        child(argc, argv, pipe_arr);
        exit(0);
    }   

    //close write end of pipe
    for (int i = 0; i < 2; i++) {
        safe_close(pipe_arr[i][1]); //close write
    }

    //crete epoll instance
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    //setup polling
    struct epoll_event event = {.events = EPOLLIN};
    for (int i = 0; i < 2; i++) {
        event.data.fd = pipe_arr[i][0]; //setup read end of pipe as the polling object
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_arr[i][0], &event);
    }

    //polling and analyzing
    struct epoll_event events[2];
    int nfds, pipes_opened = 2;
    while (pipes_opened > 0) {
        nfds = epoll_wait(epoll_fd, events, 2, -1);
        if (nfds < 0) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        //analyzing read bytes
        int process_state = -1;
        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd; 
            if (fd == pipe_arr[0][0]) {
                process_state = process_pipe(pipe_arr[0][0], ans[0]);
            }
            else if (fd == pipe_arr[1][0]) {
                process_state =  process_pipe(pipe_arr[1][0], ans[1]);
            }

            if (process_state == 0) {
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);                
                pipes_opened--;
                safe_close(fd);
                break;
            }
        }
    }

    //waiting for ending
    wait(NULL);

    for (int i = 0; i < 2; i++) {
        printf("bytes: %-3ld words: %-3ld lines: %-3ld\n", 
                ans[i][0], ans[i][1], ans[i][2]);
    }

    //close epoll instance
    safe_close(epoll_fd);

    return 0;
}

void child(int argc, char* argv[], int pipe_arr[][2]) 
{
    for (int i = 0; i < 2; i++) {
        close(pipe_arr[i][0]); //close read
    }

    if (dup2(pipe_arr[0][1], STDOUT_FILENO) < 0) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    safe_close(pipe_arr[0][1]); //close second pipe writing end

    if (dup2(pipe_arr[1][1], STDERR_FILENO) < 0) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    safe_close(pipe_arr[1][1]); //close second pipe writing end


    if (execvp((const char*)argv[1], &(argv[1])) == -1) {
        perror("execvp");
        exit(EXIT_FAILURE);
    }
}


#define BUF_SZ 4096
int process_pipe(int fd_pipe_out, ssize_t* ans)
{
    char buf[BUF_SZ];
    ssize_t bytes_sum = 0;

    ssize_t n = 0, read_bytes = 0;
    while (1) {   
        n = read(fd_pipe_out, buf + read_bytes, BUF_SZ - read_bytes);

        if (n < 0) {
            if (errno == EINTR) {
                n = 0;
                continue;
            }
            else {
                perror("error in read syscall");
                exit(EXIT_FAILURE);
            }
        }
        else if (n == 0) { //EOF
            break;
        }

        read_bytes += n;
    }

    process_buf(buf, read_bytes, ans);
    ans[0] += (ssize_t)read_bytes;

    return n;
}
#undef BUF_SZ


void process_buf(char* buf, ssize_t size, ssize_t* ans)
{
    ssize_t word_count = 0;
    ssize_t line_count = 0;
    bool in_word = false;

for (ssize_t i = 0; i < size; i++)
{
    if (buf[i] == '\n') {
        line_count++;
    }

    if (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n') {
        if (in_word) {
            word_count++;
            in_word = false;
        }
    }
    else if (!in_word) {
        in_word = true;
    }
} 

    ans[1] += word_count;
    ans[2] += line_count;
}
