#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>

#define SEM_SERVER_NAME "/server-free"
#define SHM_COMMUNICATION_SLOT_NAME "/communication-slot"

enum operation_t {
    MIN,
    MAX
};

struct communication_slot_t {
    char data_name[30];
    enum operation_t operation;
    size_t size;
    int32_t result;

    sem_t sem_client_ready;
    sem_t sem_server_done;

    pid_t server_pid;
};

#endif //COMMON_H
