#include "hive_manager.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#define PROJECT_ID 20

#define MIN_EGG_LAY_TIME 5 //Czas wylęgania jednego jajka
#define MAX_EGG_LAY_TIME 15 //Czas wylęgania jednego jajka
Hive* hive;
int shm_id;
int other_sem_id;
char log_msg[100]; 

void handle_sigchld(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
    signal(SIGCHLD, handle_sigchld);
}

void lay_egg(){
    int egg_lay_time = rand() % (MAX_EGG_LAY_TIME - MIN_EGG_LAY_TIME + 1) + MIN_EGG_LAY_TIME;
    usleep(egg_lay_time*MICROSECOND);
    semaphore_lock(other_sem_id,HIVE_STRUCTURE_SEM_IDX);
    if (hive->current_bee_population >= hive->max_bees_population){
        snprintf(log_msg,sizeof(log_msg),"Queen bee failed to lay an egg. Population is maximal now.\n");
        log_message(log_msg);
        semaphore_unlock(other_sem_id,HIVE_STRUCTURE_SEM_IDX);
        return;
    }
    if (hive->bees_in_hive < hive->max_bees_population/2){
        
       snprintf(log_msg,sizeof(log_msg),"Queen bee laid an egg\n");
       log_message(log_msg);
        hive->bees_in_hive+=1;
        hive->current_bee_population+=1;
        pid_t bee_pid = fork();
        if (bee_pid == 0){
            execl("./workerBee","workerBee","egg","&",NULL);
            perror("Failed to exec workerBee");
            exit(EXIT_FAILURE);
        }
        else if (bee_pid == -1){
            perror("Failed to fork bee");
            exit(EXIT_FAILURE);
        }
        
    }
    else {
        snprintf(log_msg,sizeof(log_msg),"Queen bee failed to lay an egg. Hive is overpopulated now.\n");
        log_message(log_msg);
    }
    semaphore_unlock(other_sem_id,HIVE_STRUCTURE_SEM_IDX);
}
void cleanup(int signum) {
    detach_from_hive(hive);
    snprintf(log_msg,sizeof(log_msg),"QueenBee:%d cleaned up resources and is exiting.\n", getpid());
    //log_message(log_msg);
    exit(0);
}

void setup(){
    setpgid(0, getppid());
    signal(SIGINT, cleanup);
    signal(SIGCHLD, handle_sigchld);
    shm_id = init_shared_hive();
    hive = attach_to_hive(shm_id);
    other_sem_id = get_other_semaphores();
}


int main(){
   setup();
    while(1){
        lay_egg();
    }
    return 0;
}