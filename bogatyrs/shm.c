#include "shm.h"

#define SHMEM_SIZE 4096
 
void make_shmem(const char* name, int* ret_fd, void** ret_ptr)
{
    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, SHMEM_SIZE);
    void *ptr = mmap(NULL, SHMEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
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