#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#ifndef SHMEM_HEADER
#define SHMEM_EDER

void make_shmem(const char* name, int* ret_fd, void** ret_ptr);

void join_shmem(const char* name, int* ret_fd, void** ret_ptr);

void close_shmem(void* shm_ptr, int shm_fd, const char* shm_name);

int get_db_size(void* db);

void add_pid_to_db(void* pid_db, int pid);

int get_pid_from_db(void* pid_db, int pid_num);

#endif
