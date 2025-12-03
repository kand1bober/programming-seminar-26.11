#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>

#define _POSIX_C_SOURCE 200809L

#include "mqueue.h"
#include "shm.h"
#include "bog.h"

#define kBogatyrsNum 128

#define kElectionTimeMin 150 //in msec
#define kElectionTimeMax 300 //in msec

const char* kShmemName = "/Bogatyrs_shmem";

typedef struct {
    int election_done;        // 0 = не завершены, 1 = завершены
    pid_t leader_pid;         // PID лидера
    pthread_mutex_t mutex;    // мьютекс для защиты состояния
    pid_t pids[kBogatyrsNum];
    int count;
} SharedState;

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

    //make msg_queue
    int qid = msgget(IPC_PRIVATE, IPC_CREAT | 0666); //create msg_queue
    if (qid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    //make bogatyrs prcoesses
    pid_t pid;
    for (int i = 0; i < kBogatyrsNum; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) { //child
            bogatyr(qid);
            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < kBogatyrsNum; i++) {
        wait(NULL);
    }

    //close shmem
    close_shmem(shmem_ptr, shmem_fd, kShmemName);

    if (msgctl(qid, IPC_RMID, 0) == -1){
        perror("msgctl");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void bogatyr(int qid)
{
    BogatyrInfo bog_info = {.election_time = random_in_range(kElectionTimeMin, kElectionTimeMax), 
                            .proc_role = kFollower, 
                            .self_pid = getpid(),                             
                            .qid = qid};

    sprintf(bog_info.shm_name, "%s", kShmemName);

    join_shmem(bog_info.shm_name, &bog_info.shm_fd, &bog_info.shm_ptr); 
    add_pid_to_db(bog_info.shm_ptr, bog_info.self_pid);
    sprintf(bog_info.str, "%s", (char*)bog_info.shm_ptr); //save string to sing

    make_election(&bog_info);

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

void make_election(BogatyrInfo* bog_info)
{
    bog_info->proc_role = kFollower;

    //create mask
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, kRequestVote);

    //wait for kRequest signal from candidate
    siginfo_t info;
    union sigval data;

    printf("election time in nanosec: %f\n", bog_info->election_time * 1e6);
    struct timespec time = {.tv_nsec = bog_info->election_time * 1e6};
    if (sigtimedwait(&set, &info, &time) < 0) {
        if (errno == EAGAIN) {
            //became candidate, init voting
            bog_info->proc_role = kCandidate;

            for (int i = 0; i < get_db_size(bog_info->shm_ptr); i++) {
                data.sival_int = bog_info->self_pid;
                sigqueue(get_pid_from_db(bog_info->shm_ptr, i), kRequestVote, data); //send msg with self pid in info

                if ((collect_votes(bog_info) + 1) > (kBogatyrsNum / 2)) { //election is won
                    printf("Won election\n");
                    bog_info->proc_role = kLeader;
                    return;
                }
                else { //election is lost
                    printf("Lost election\n");
                    make_election(bog_info); //make reelection
                    return;
                }
            }
        }
        else {
            perror("sigtimedwait");
            exit(EXIT_FAILURE);
        }
    }
    else { //signal received => there is already a candidate => vote for him
        if (info.si_value.sival_int >= 0) {
            printf("voted\n");
            data.sival_int = kAffirmative;
            sigqueue(info.si_value.sival_int, kRequestVote, data);
        }
    }
}

int collect_votes(BogatyrInfo* bog_info)
{
    int votes = 0;

    //create mask
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, kRequestVote);

    //wait for kRequest signal from candidate
    siginfo_t info;
    struct timespec time = {.tv_nsec = 5000};
    
    for (int i = 0; i < get_db_size(bog_info->shm_ptr) - 1; i++) {
        if (sigtimedwait(&set, &info, &time) < 0) {
            if (errno != EAGAIN) {
                perror("sigtimedwait");
                exit(EXIT_FAILURE);
            }
        }
        else { //signal received => vote counted
            if (info.si_value.sival_int == kAffirmative) {
                votes++;
            }
        }
    }

    return votes;
}

void follower(BogatyrInfo* bog_info)
{   
    printf("%d: follower\n", getpid());
}

void candidate(BogatyrInfo* bog_info)
{
    printf("%d: candidate\n", getpid());

}

void leader(BogatyrInfo* bog_info)
{
    printf("%d: leader\n", getpid());

}

int random_in_range(int min, int max)
{   
    return min + rand() % (max - min + 1);
}
