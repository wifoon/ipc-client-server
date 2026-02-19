#include <stdio.h>
#include "common.h"

int main(int argn, char* argv[])
{
    if (argn != 3) {
        printf("Wrong number of arguments\n");
        return 1;
    }

    FILE* fp = fopen(argv[1], "r");
    if (!fp) {
        printf("File not found\n");
        return 1;
    }

    size_t tab_size = 0;
    int32_t value;
    while(fscanf(fp, "%d", &value) == 1) {
        tab_size++;
    }
    rewind(fp);

    char shm_data_name[30];
    sprintf(shm_data_name, "/shm_data_%d", getpid());

    int fd_data = shm_open(shm_data_name, O_CREAT | O_RDWR | O_EXCL, 0666);
    ftruncate(fd_data, tab_size * sizeof(int32_t));
    int32_t* data = mmap(NULL, tab_size * sizeof(int32_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd_data, 0);

    int i = 0;
    while(fscanf(fp, "%d", &value) == 1) {
        data[i++] = value;
    }

    fclose(fp);

    sem_t *sem_server = sem_open(SEM_SERVER_NAME, 0);
    if (sem_server == SEM_FAILED) {
        printf("Server not working\n");
        munmap(data, tab_size * sizeof(int32_t));
        close(fd_data);
        shm_unlink(shm_data_name);
        return 0;
    }

    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    time.tv_sec += 5;

    if (sem_timedwait(sem_server, &time) == -1) {
        printf("Server is busy, try later\n");
        sem_close(sem_server);
        munmap(data, tab_size * sizeof(int32_t));
        close(fd_data);
        shm_unlink(shm_data_name);
        return 0;
    }

    // if (sem_trywait(sem_server) == -1) {
    //     printf("Server is busy, try later\n");
    //     sem_close(sem_server);
    //     munmap(data, tab_size * sizeof(int32_t));
    //     close(fd_data);
    //     shm_unlink(shm_data_name);
    //     return 0;
    // }


    int fd = shm_open(SHM_COMMUNICATION_SLOT_NAME, O_RDWR, 0666);
    if (fd == -1) {
        printf("Server not workingCHYBAXDDDDDDD\n");
        sem_post(sem_server);
        sem_close(sem_server);
        munmap(data, tab_size * sizeof(int32_t));
        close(fd_data);
        shm_unlink(shm_data_name);
        return 0;
    }

    struct communication_slot_t* communication_slot = mmap(NULL, sizeof(struct communication_slot_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    int server_status = kill(communication_slot->server_pid, 0);
    if (server_status == -1) {
        printf("Server not working\n");
        munmap(communication_slot, sizeof(struct communication_slot_t));
        close(fd);
        sem_post(sem_server);
        sem_close(sem_server);
        munmap(data, tab_size * sizeof(int32_t));
        close(fd_data);
        shm_unlink(shm_data_name);
        return 0;
    }

    strcpy(communication_slot->data_name, shm_data_name);
    communication_slot->size = tab_size;

    if (strcasecmp((argv[2]), "min") == 0) {
        communication_slot->operation = MIN;
    } else if (strcasecmp((argv[2]), "max") == 0) {
        communication_slot->operation = MAX;
    } else {
        printf("Incorrect operation\n");
        munmap(communication_slot, sizeof(struct communication_slot_t));
        close(fd);
        sem_post(sem_server);
        sem_close(sem_server);
        munmap(data, tab_size * sizeof(int32_t));
        close(fd_data);
        shm_unlink(shm_data_name);
        return 0;
    }

    sem_post(&communication_slot->sem_client_ready);
    sem_wait(&communication_slot->sem_server_done);

    printf("Result: %d\n", communication_slot->result);


    munmap(communication_slot, sizeof(struct communication_slot_t));
    close(fd);

    sem_post(sem_server);
    sem_close(sem_server);

    munmap(data, tab_size * sizeof(int32_t));
    close(fd_data);
    shm_unlink(shm_data_name);

    return 0;
}
