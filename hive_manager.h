#ifndef HIVE_MANAGER_H
#define HIVE_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <sys/time.h>

#define N 100 //Startowa liczba pszczol
#define ENTRIES_SEMAPHORE_ID 19// id do laczenia sie z semaforami wejsc do ula
#define OTHER_IPC_ID 20 //id do laczenia sie z reszta rzeczy
#define LEFT_ENTRY_IDX 0//indeks semafora lewych dzrzwi
#define RIGHT_ENTRY_IDX 1//indeks semafora prawych drzwi
#define ENTER_HIVE_IDX 2// indeks semafora do proby wejscia
#define HIVE_STRUCTURE_SEM_IDX 0
#define MICROSECOND 10000
typedef struct {
    int bees_in_hive;
    int max_bees_population;
    int current_bee_population
} Hive;

key_t generate_ipc_key(int section_id) {
    key_t key = ftok("/tmp", section_id);
    if (key == -1) {
        perror("Failed to generate key with ftok");
        exit(EXIT_FAILURE);
    }

    return key;
}

int get_entry_semaphores() {
    int sem_id = semget(generate_ipc_key(ENTRIES_SEMAPHORE_ID), 3, 0622 | IPC_CREAT);
    if (sem_id == -1) {
        perror("semget failed for entry semaphores");
        exit(1);
    }
    return sem_id;
}
int get_other_semaphores(){
    int sem_id = semget(generate_ipc_key(OTHER_IPC_ID), 1, 0622 | IPC_CREAT);
    if (sem_id == -1) {
        perror("semget failed for other semaphores");
        exit(1);
    }
    return sem_id;
}

void semaphore_lock(int sem_id, int sem_index) {
    struct sembuf sb = {sem_index, -1, 0}; 
    if (semop(sem_id, &sb, 1) == -1) {
        perror("semop lock failed");
        exit(1);
    }
}
void semaphore_lock_no_wait(int sem_id, int sem_index) {
    struct sembuf sb = {sem_index, -1, IPC_NOWAIT}; 
    if (semop(sem_id, &sb, 1) == -1) {
        perror("semop lock failed");
        exit(1);
    }
}
void semaphore_unlock(int sem_id, int sem_index) {
    struct sembuf sb = {sem_index, 1, 0};
    if (semop(sem_id, &sb, 1) == -1) {
        perror("semop unlock failedx");
        exit(1);
    }
}

int init_shared_hive(){
    int shm_id = shmget(generate_ipc_key(OTHER_IPC_ID), sizeof(Hive), IPC_CREAT | 0622);
    if (shm_id == -1) {
        perror("Failed to create shared memory");
        exit(EXIT_FAILURE);
    
}
return shm_id;
}

Hive* attach_to_hive(int shm_id)
{
    Hive* hive = (Hive*) shmat(shm_id, NULL, 0);
    if (hive == (void*) -1) {
        perror("Failed to attach shared memory");
        exit(EXIT_FAILURE);
    }
    return hive;
}
void detach_from_hive(void *hive) {
    if (shmdt(hive) == -1) {
        perror("Failed to detach shared memory");
    } 
}    

void log_message(char* message){
    printf("%s", message);
    FILE* log_file = fopen("log.txt","a");
    if (log_file == NULL){
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }
     struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec;
    struct tm* tm_info = localtime(&now);
    char time_str[30];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    snprintf(time_str + strlen(time_str), sizeof(time_str) - strlen(time_str), ".%03ld", tv.tv_usec / 1000);
    fprintf(log_file, "[%s] %s", time_str, message);
    fflush(log_file);
    fclose(log_file);
}

#endif