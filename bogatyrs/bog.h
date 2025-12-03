#ifndef BOGATYRS_HEADER
#define BOGATYRSHEADER

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

    //text info
    int self_letter;
    char str[128]; //full string to sing

    //shmem info
    //shmem is used for making data base of pids
    void* shm_ptr;
    int shm_fd;
    char shm_name[128];

    //msg_queue info
    int qid;
} BogatyrInfo;

void bogatyr(int qid);

int random_in_range(int min, int max);

int collect_votes(BogatyrInfo* bog_info);

void make_election(BogatyrInfo* bog_info);

void follower(BogatyrInfo* bog_info);

void candidate(BogatyrInfo* bog_info);

void leader(BogatyrInfo* bog_info);

#endif
