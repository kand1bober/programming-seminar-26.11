#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

#include "shm.h"

#define kBogatyrsNum 128

#define kElectionTimeMin 150 //in msec
#define kElectionTimeMax 300 //in msec

#define kHeartbeatTimeMin 10 //in msec
#define kHeartbeatTimeMax 50 //in msec

typedef enum {
    kFollower,
    kCandidate,
    kLeader,
} ProcessRole;

typedef struct {
    //consensus info
    int term;
    ProcessRole proc_role;
    int self_pid;
    int election_time;
    int heartbeat_time;

    //text info
    int self_letter;
    // const char* 

    //shmem info
    void* shm_ptr;
    int shm_fd;
    const char* shm_name;
} BogatyrInfo;

int random_in_range(int min, int max);

const char* kShmemName = "Bogatyrs_shmem";

int main(int argc, char* argv[])
{   
    srand((unsigned int)time(NULL)); //initialize generator of random numbers

    char str[128];
    scanf("%s", str);

    //make shmem and put string in it
    void* shmem_ptr = NULL;
    int shmem_fd;
    make_shmem(kShmemName, &shmem_fd, &shmem_ptr);
    sprintf(shmem_ptr, "%s", str);

    //make bogatyrs prcoesses
    for (int i = 0; i < kBogatyrsNum; i++) {
        pid_t pid;
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) { //child
            bogatyr(str);
        }
    }

    for (int i = 0; i < kBogatyrsNum; i++) {
        wait(NULL);
    }

    //close shmem
    close_shmem(shmem_ptr, shmem_fd, kShmemName);

    return 0;
}

void bogatyr()
{
    BogatyrInfo bog_info = {.election_time = random_in_range(kElectionTimeMin, kElectionTimeMax), 
                            .heartbeat_time = random_in_range(kHeartbeatTimeMin, kHeartbeatTimeMax), 
                            .proc_role = kFollower, 
                            .self_pid = getpid(), 
                            
                            .shm_name = kShmemName};

    join_shmem(bog_info.shm_name, &bog_info.shm_fd, &bog_info.shm_ptr);

    while (1) {
        switch (bog_info.proc_role) {
            case kFollower: {
                follower(&bog_info);
                break;
            }
            case kCandidate: {
                candidate(&bog_info);
                break;
            }
            case kLeader: {
                leader(&bog_info);
                break;
            }
        } 
    }

    // while (1) {
        //sigwaitinfo(kTermTimeOut); //подождать сигнал от лидера

        //обработать полученный сигнал
        // if (no signal) {
            // become candidate
            // init voting
        // }
        // else if (signal.info == self_letter) {
        //     send_to_leader(false);
        // }
    // }

}

void follower(BogatyrInfo* bog_info)
{

}

void candidate(BogatyrInfo* bog_info)
{

}

void leader(BogatyrInfo* bog_info)
{

}

int random_in_range(int min, int max)
{   
    return min + rand() % (max - min + 1);
}