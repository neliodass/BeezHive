#include "hive_manager.h"

#define PROJECT_ID 20
#define EGG_LAY_TIME 60 //Czas wylęgania jednego jajka


int main(){
    int shm_id = shmget(generate_ipc_key(PROJECT_ID), sizeof(Hive), 0662);
    Hive* hive = (Hive*)shmat(shm_id, NULL, 0);
    int sem_id = semget(generate_ipc_key(PROJECT_ID),)

}