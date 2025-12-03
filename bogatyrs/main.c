#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <pthread.h>

#include "mqueue.h"
#include "shm.h"
#include "bog.h"

#define kElectionTimeMin 150 //in msec
#define kElectionTimeMax 300 //in msec

const char* kShmemName = "/Bogatyrs_shmem";

int main(int argc, char* argv[])
{   
    srand((unsigned int)time(NULL)); //initialize generator of random numbers

    char str[128];
    scanf("%s", str);

    //make shm
    SharedState* state = NULL; //shm pointer
    int shm_fd;
    make_shmem(kShmemName, &shm_fd, (void**)&state);

    //make bogatyr prcoesses
    pid_t pid;
    for (int i = 0; i < kBogatyrsNum; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) { //child
            bogatyr();
            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < kBogatyrsNum; i++) {
        wait(NULL);
    }

    //close shmem
    close_shmem(state, shm_fd, kShmemName);

    return 0;
}

void bogatyr()
{
    SharedState* state = NULL;
    int shm_fd;
    join_shmem(kShmemName, &shm_fd, (void**)&state);

    pid_t my_pid = getpid();

    // register ourself in base (under mutex)
    pthread_mutex_lock(&state->mutex);
    int my_index = state->count;
    if (my_index < kBogatyrsNum) {
        state->pids[my_index] = my_pid;
        state->count++;
    }
    int election_done = state->election_done;
    pthread_mutex_unlock(&state->mutex);

    // check election state
    if (election_done) {
        printf("%d: follower (leader=%d)\n", my_pid, state->leader_pid);
        return;
    }

    // wait before becoming a candidate on election
    int us = random_in_range(kElectionTimeMin, kElectionTimeMax) * 1e3;
    usleep(us);

    // check election state
    pthread_mutex_lock(&state->mutex);
    if (state->election_done) { //leader chosen
        printf("%d: follower (leader=%d)\n", my_pid, state->leader_pid);
    }
    else { //no leader => become leader
        state->leader_pid = my_pid;
        state->election_done = 1;
        printf("%d: LEADER\n", my_pid);
    }
    pthread_mutex_unlock(&state->mutex);

    return;
}

int random_in_range(int min, int max)
{   
    return min + rand() % (max - min + 1);
}
