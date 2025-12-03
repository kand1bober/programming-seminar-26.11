#include "shm.h"

#define SHMEM_SIZE 4096
 
void make_shmem(const char* name, int* ret_fd, void** ret_ptr)
{
    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, SHMEM_SIZE);
    void *ptr = mmap(NULL, SHMEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    memset(ptr, 0, SHMEM_SIZE);

    *ret_fd = fd;
    *ret_ptr = ptr;
}   

void join_shmem(const char* name, int* ret_fd, void** ret_ptr)
{       
    int fd = shm_open(name, O_RDWR, 0666); //open already existing shmem
    *ret_fd = fd;

    void* ptr = mmap(NULL, SHMEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    *ret_ptr = ptr;
}

void close_shmem(void* shm_ptr, int shm_fd, const char* shm_name)
{
    munmap(shm_ptr, SHMEM_SIZE);
    close(shm_fd);

    shm_unlink(shm_name);
}

int get_db_size(void* db)
{
    int num;
    memcpy(&num, db, sizeof(int)); //get pid counter

    return num;
}

void add_pid_to_db(void* pid_db, int pid)
{
    int num = get_db_size(pid_db);

    memcpy((int*)pid_db + num + 1, &pid, sizeof(int)); //add new pid

    // printf("add: num = %d, to_add = %d, result = %d\n", num, pid, *((int*)pid_db + num));
 
    num++;
    memcpy(pid_db, &num, sizeof(int)); //increase pid counter
}

int get_pid_from_db(void* pid_db, int pid_num)
{
    int num = get_db_size(pid_db);

    if (pid_num > num) {
        printf("invalid number of pid to take from pid data base\n");
        exit(EXIT_FAILURE);
    }

    int res;
    memcpy(&res, (int*)pid_db + (pid_num + 1), sizeof(int));
    return res; 
}

// void show_all_pids(ChatInfo* chat_info)
// {
//     int db_size;
//     memcpy(&db_size, chat_info->pid_db, sizeof(int));

//     printf("list of user pids:\n");
//     for (int i = 0; i < db_size; i++) {
//         printf("%2d: %d\n", i, get_pid_from_db(chat_info->pid_db, i));
//     }
//     printf("\n");
// }

