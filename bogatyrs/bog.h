#ifndef BOGATYRS_HEADER
#define BOGATYRSHEADER

#define kBogatyrsNum 128

typedef struct {
    int election_done;        // 0 = не завершены, 1 = завершены
    pid_t leader_pid;         // PID лидера
    pthread_mutex_t mutex;    // мьютекс для защиты состояния
    pid_t pids[kBogatyrsNum];
    int count;
} SharedState;

void bogatyr();

int random_in_range(int min, int max);

#endif
