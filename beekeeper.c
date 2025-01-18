#include "hive_manager.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
Hive* hive;
int shm_id;
int entry_sem_id,other_sem_id;

char log_msg[100];
char start_time_str[30];

void handle_child_termination(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
    signal(SIGCHLD, handle_child_termination);
}
void rename_log_file() {
    char new_log_file_name[50];
    snprintf(new_log_file_name, sizeof(new_log_file_name), "logs/log_%s.txt", start_time_str);
    if (rename("log.txt", new_log_file_name) != 0) {
        perror("Failed to rename log file");
    }
}
void spawnQueen()
{
    pid_t queen_pid = fork();
    if (queen_pid == 0){
        execl("./queenBee","queenBee","&",NULL);
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
        execl("./workerBee","workerBee","adult","&",NULL);
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

    shm_id = init_shared_hive();
    hive = attach_to_hive(shm_id);
    hive->max_bees_population = N;
    hive->bees_in_hive = 0;
    hive->current_bee_population = 0;
    entry_sem_id = get_entry_semaphores();
    other_sem_id = get_other_semaphores();
    semctl(entry_sem_id,LEFT_ENTRY_IDX,SETVAL,1);   
    semctl(entry_sem_id,RIGHT_ENTRY_IDX,SETVAL,1);
    semctl(other_sem_id,HIVE_STRUCTURE_SEM_IDX,SETVAL,1);
    semaphore_lock(other_sem_id,HIVE_STRUCTURE_SEM_IDX);
    spawnQueen();
    hive->bees_in_hive+=1;
    hive->current_bee_population+=1;
    for (int i = 0; i < N-1; i++){
        spawnWorkerBee();
        hive->bees_in_hive+=1;
        hive->current_bee_population+=1;
    }
    semaphore_unlock(other_sem_id,HIVE_STRUCTURE_SEM_IDX);
    snprintf(log_msg,sizeof(log_msg),"Created queen and worker bees\n");
    log_message(log_msg);
}
void increase_capacity(int signum) {
    semaphore_lock(other_sem_id, HIVE_STRUCTURE_SEM_IDX);
    hive->max_bees_population = 2 * N;
    snprintf(log_msg,sizeof(log_msg),"Beekeeper increased hive capacity to %d\n", hive->max_bees_population);
    log_message(log_msg);
    semaphore_unlock(other_sem_id, HIVE_STRUCTURE_SEM_IDX);
    signal(SIGUSR1, increase_capacity);
}
void decrease_capacity(int signum) {
    semaphore_lock(other_sem_id, HIVE_STRUCTURE_SEM_IDX);
    if (hive->max_bees_population < 2) {
        snprintf(log_msg,sizeof(log_msg),"Hive capacity cannot be decreased further\n");
        log_message(log_msg);
        semaphore_unlock(other_sem_id, HIVE_STRUCTURE_SEM_IDX);
        signal(SIGUSR2, decrease_capacity);
        return;
    }
    hive->max_bees_population = hive->max_bees_population / 2;
    snprintf(log_msg,sizeof(log_msg),"Beekeeper decreased hive capacity to %d\n", hive->max_bees_population);
    log_message(log_msg);
    semaphore_unlock(other_sem_id, HIVE_STRUCTURE_SEM_IDX);
    signal(SIGUSR2, decrease_capacity);
    
}
void cleanup(int signum) {
    signal(SIGINT, SIG_IGN);
    killpg(getpgrp(), SIGINT);
    sleep(5);
    detach_from_hive(hive);
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID failed");
    }
    if (semctl(entry_sem_id, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID failed for entry_sem_id");
    }
    if (semctl(other_sem_id, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID failed for other_sem_id");
    }
    snprintf(log_msg,sizeof(log_msg),"Beekeeper cleaned up resources.\n");
    log_message(log_msg);
    rename_log_file();
    exit(0);
}

int main()
{
    signal(SIGUSR1, increase_capacity);
    signal(SIGUSR2, decrease_capacity);
    signal(SIGINT, cleanup);
    signal(SIGCHLD, handle_child_termination);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec;
    struct tm* tm_info = localtime(&now);
    strftime(start_time_str, sizeof(start_time_str), "%Y-%m-%d_%H-%M-%S", tm_info);
    snprintf(log_msg,sizeof(log_msg),"Beekeeper is running. Send SIGUSR1 to increase capacity, SIGUSR2 to decrease capacity, SIGINT to clean up and exit.\n");
    log_message(log_msg);
    setupHive();
    while(1){
        pause();
    }
}

