#include "hive_manager.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
Hive* hive;
int shm_id;
int entry_sem_id,other_sem_id;





void spawnQueen()
{
    pid_t queen_pid = fork();
    if (queen_pid == 0){
        execl("./queenBee","queenBee",NULL);
        perror("Failed to exec queenBee");
        exit(EXIT_FAILURE);
    }
    else if (queen_pid == -1){
        perror("Failed to fork queen");
        exit(EXIT_FAILURE);
    }
}
void spawnWorkerBee()
{
    pid_t bee_pid = fork();
    if (bee_pid == 0){
        execl("./workerBee","workerBee",NULL);
        perror("Failed to exec workerBee");
        exit(EXIT_FAILURE);
    }
    else if (bee_pid == -1){
        perror("Failed to fork bee");
        exit(EXIT_FAILURE);
    }
}

void setupHive()
{
    //TODO: Ustawic startowa ilosc pszczol w ulu!
    shm_id = init_shared_hive();
    hive = attach_to_hive(shm_id);
    hive->bees_population = N;
    hive->bees_in_hive = 0;
    entry_sem_id = get_entry_semaphores();
    other_sem_id = get_other_semaphores();
    semctl(entry_sem_id,LEFT_ENTRY_IDX,SETVAL,1);   
    semctl(entry_sem_id,RIGHT_ENTRY_IDX,SETVAL,1);
    semctl(entry_sem_id,ENTER_HIVE_IDX,SETVAL,0);
    semctl(other_sem_id,HIVE_STRUCTURE_SEM_IDX,SETVAL,1);
    semaphore_lock(other_sem_id,HIVE_STRUCTURE_SEM_IDX);
    spawnQueen();
    hive->bees_in_hive+=1;
    for (int i = 0; i < N-1; i++){
        spawnWorkerBee();
        hive->bees_in_hive+=1;
    }
    semaphore_unlock(other_sem_id,ENTER_HIVE_IDX);
}
void increase_capacity(int signum) {
    semaphore_lock(other_sem_id, HIVE_STRUCTURE_SEM_IDX);
    hive->bees_population = 2 * N;
    printf("Beekeeper increased hive capacity to %d\n", hive->bees_population);
    semaphore_unlock(other_sem_id, HIVE_STRUCTURE_SEM_IDX);
}
void decrease_capacity(int signum) {
    semaphore_lock(other_sem_id, HIVE_STRUCTURE_SEM_IDX);
    hive->bees_population = hive->bees_population / 2;
    printf("Beekeeper decreased hive capacity to %d\n", hive->bees_population);
    semaphore_unlock(other_sem_id, HIVE_STRUCTURE_SEM_IDX);
}
void cleanup(int signum) {
    killpg(getpgrp(), SIGINT);
    detach_from_hive(hive);
    semctl(entry_sem_id, 0, IPC_RMID);
    semctl(other_sem_id, 0, IPC_RMID);
    printf("Beekeeper cleaned up resources.\n");
    exit(0);
}
int main()
{
    signal(SIGUSR1, increase_capacity);
    signal(SIGUSR2, decrease_capacity);
    signal(SIGINT, cleanup);
    printf("Beekeeper is running. Send SIGUSR1 to increase capacity, SIGUSR2 to decrease capacity, SIGINT to clean up and exit.\n");
    setupHive();
    while(1){
        pause();
    }
}

