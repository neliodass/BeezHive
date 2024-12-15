#include "hive_manager.h"

Hive* hive;
int shm_id;
int entry_sem_id,other_sem_id;

void setupHive()
{
    //TODO: Ustawic startowa ilosc pszczol w ulu!
    shm_id = init_shared_hive();
    hive = attach_to_hive(shm_id);
    hive->bees_population = N;
    entry_sem_id = get_entry_semaphores();
    other_sem_id = get_other_semaphores();
    semctl(entry_sem_id,LEFT_ENTRY_IDX,SETVAL,1);   
    semctl(entry_sem_id,RIGHT_ENTRY_IDX,SETVAL,1);
    semctl(entry_sem_id,ENTER_HIVE_IDX,SETVAL,0);
    semctl(other_sem_id,HIVE_STRUCTURE_SEM_IDX,SETVAL,1);
    
}

int main()
{
    
}

