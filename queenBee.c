#include "hive_manager.h"
#include <signal.h>
#define PROJECT_ID 20
#define EGG_LAY_TIME 180 //Czas wylęgania jednego jajka
Hive* hive;
int shm_id;
int other_sem_id;

void lay_egg(){
    sleep(EGG_LAY_TIME);
    semaphore_lock(other_sem_id,HIVE_STRUCTURE_SEM_IDX);
    if (hive->bees_in_hive/2 < hive->bees_population){
        
        printf("Queen bee laid an egg\n");
        hive->bees_in_hive+=1;
        pid_t bee_pid = fork();
        if (bee_pid == 0){
            execl("./workerBee","workerBee","egg",NULL);
            perror("Failed to exec workerBee");
            exit(EXIT_FAILURE);
        }
        else if (bee_pid == -1){
            perror("Failed to fork bee");
            exit(EXIT_FAILURE);
        }
        
    }
    printf("Queen bee failed to lay an egg. Hive is overpopulated now.\n");
    semaphore_unlock(other_sem_id,ENTER_HIVE_IDX);
}
void cleanup(int signum) {
    detach_from_hive(hive);
    printf("Pszczola:%d cleaned up resources and is exiting.\n", getpid());
    exit(0);
}

void setup(){
    setpgid(0, getppid());
    signal(SIGINT, cleanup);
    srand(time(NULL));
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