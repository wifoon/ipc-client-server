#include <stdio.h>
#include "common.h"

sem_t* sem_server;

struct communication_slot_t* communication_slot;

int stat_min = 0;
int stat_max = 0;
pthread_mutex_t stat_mutex = PTHREAD_MUTEX_INITIALIZER;

int terminate = 0;
pthread_mutex_t terminate_mutex = PTHREAD_MUTEX_INITIALIZER;

void* timer_th(void* arg) {
    while(1) {
        pthread_mutex_lock(&terminate_mutex);
        if(terminate) {
            pthread_mutex_unlock(&terminate_mutex);
            break;
        }
        pthread_mutex_unlock(&terminate_mutex);

        sleep(10);

        pthread_mutex_lock(&stat_mutex);
        printf("Ilosc zapytan: %d\n", stat_min + stat_max);
        pthread_mutex_unlock(&stat_mutex);
    }
    return NULL;
}

void* commands_ui_th(void* arg) {
    while(1) {
        char command[30];
        scanf("%s", command);

        if(strcasecmp(command, "quit") == 0) {
            pthread_mutex_lock(&terminate_mutex);
            terminate = 1;
            pthread_mutex_unlock(&terminate_mutex);
            sem_post(&communication_slot->sem_client_ready);
            break;
        } else if (strcasecmp(command, "stat") == 0) {
            pthread_mutex_lock(&stat_mutex);
            printf("Min = %d, Max = %d\n", stat_min, stat_max);
            pthread_mutex_unlock(&stat_mutex);
        } else if (strcasecmp(command, "reset") == 0) {
            pthread_mutex_lock(&stat_mutex);
            stat_min = stat_max = 0;
            pthread_mutex_unlock(&stat_mutex);
        } else continue;
    }
    return NULL;
}

void* operations_th(void* arg) {
    while(1) {
        sem_wait(&communication_slot->sem_client_ready);

        sleep(7);

        pthread_mutex_lock(&terminate_mutex);
        if(terminate) {
            pthread_mutex_unlock(&terminate_mutex);
            break;
        }
        pthread_mutex_unlock(&terminate_mutex);

        int fd_data = shm_open(communication_slot->data_name, O_RDWR, 0666);
        int32_t* data = mmap(NULL, communication_slot->size * sizeof(int32_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd_data, 0);

        int32_t res = data[0];
        switch(communication_slot->operation) {
            case MIN:
                for(size_t i = 0; i < communication_slot->size; i++) {
                    if (data[i] < res) res = data[i];
                }
                communication_slot->result = res;
                pthread_mutex_lock(&stat_mutex);
                stat_min++;
                pthread_mutex_unlock(&stat_mutex);
                break;
            case MAX:
                for(size_t i = 0; i < communication_slot->size; i++) {
                    if (data[i] > res) res = data[i];
                }
                communication_slot->result = res;
                pthread_mutex_lock(&stat_mutex);
                stat_max++;
                pthread_mutex_unlock(&stat_mutex);
                break;
        }

        munmap(data, communication_slot->size * sizeof(int32_t));
        close(fd_data);

        sem_post(&communication_slot->sem_server_done);
    }
    return NULL;
}

int main(void)
{
    printf("Enter operation: (stat/reset/quit)");

    sem_unlink(SEM_SERVER_NAME);
    shm_unlink(SHM_COMMUNICATION_SLOT_NAME);

    sem_server = sem_open(SEM_SERVER_NAME, O_CREAT | O_EXCL | O_RDWR, 0666, 0);

    int fd = shm_open(SHM_COMMUNICATION_SLOT_NAME, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (fd == -1) {
        printf("Server start error\n");
        sem_close(sem_server);
        sem_unlink(SEM_SERVER_NAME);
        sem_unlink(SHM_COMMUNICATION_SLOT_NAME);
        return 1;
    }

    ftruncate(fd, sizeof(struct communication_slot_t));
    communication_slot = mmap(NULL, sizeof(struct communication_slot_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    sem_init(&communication_slot->sem_client_ready, 1, 0);
    sem_init(&communication_slot->sem_server_done, 1, 0);

    communication_slot->server_pid = getpid();

    pthread_t th_command, th_operation, th_timer;
    pthread_create(&th_command, NULL, commands_ui_th, NULL);
    pthread_create(&th_operation, NULL, operations_th, NULL);
    pthread_create(&th_timer, NULL, timer_th, NULL);

    sem_post(sem_server);

    pthread_join(th_command, NULL);
    pthread_join(th_operation, NULL);
    pthread_join(th_timer, NULL);

    sem_destroy(&communication_slot->sem_client_ready);
    sem_destroy(&communication_slot->sem_server_done);

    sem_close(sem_server);
    sem_unlink(SEM_SERVER_NAME);

    munmap(communication_slot, sizeof(struct communication_slot_t));
    close(fd);
    shm_unlink(SHM_COMMUNICATION_SLOT_NAME);

    return 0;
}
