#include "hive_manager.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define WORKER_LIFE_TIME 5 //Czas życia jednej pszczoły(liczba odwiedzin w ulu)
#define OVERHEAT_TIME 600 //Czas zanim pszczoła się przegrzeje
#define MIN_INCUB_TIME 50
#define MAX_INCUB_TIME 180
#define MAX_OUT_TIME 400
#define MIN_OUT_TIME 100
bool in_hive=true;
int entry_count = 0;
Hive* hive;
int shm_id;
int entry_sem_id,other_sem_id;


void incubation()
{
int incubation_time = rand()%(MAX_INCUB_TIME-MIN_INCUB_TIME+1)+MIN_INCUB_TIME;
sleep(incubation_time);
bee_process();
}
int random_door() {
    return rand() % 2;
}
void leave_hive()
{
int door_num = random_door();
semaphore_lock(entry_sem_id,door_num);
semaphore_lock(other_sem_id,HIVE_STRUCTURE_SEM_IDX);
hive->bees_in_hive-=1;
in_hive = false;
if (hive->bees_in_hive<hive->bees_population/2) semaphore_unlock(entry_sem_id,ENTER_HIVE_IDX); 
printf("Pszczola:%d opuscila ul. Liczba pszczol w ulu:%d/%d\n",getpid(),hive->bees_in_hive,hive->bees_population);
semaphore_unlock(other_sem_id,HIVE_STRUCTURE_SEM_IDX);
semaphore_unlock(entry_sem_id,door_num);

}
void stay_outside()
{
    int outside_time = rand()%(MAX_OUT_TIME-MIN_OUT_TIME+1)+MIN_OUT_TIME;
    sleep(outside_time);
}
void enter_hive()
{
    int door_num = random_door();
    semaphore_lock(entry_sem_id,ENTER_HIVE_IDX);
    semaphore_lock(entry_sem_id,door_num);
    semaphore_lock(other_sem_id,HIVE_STRUCTURE_SEM_IDX);
    hive->bees_in_hive+=1;
    in_hive = true;
    entry_count+=1;
    printf("Pszczola:%d weszla do ula. Liczba pszczol w ulu:%d/%d\n",getpid(),hive->bees_in_hive,hive->bees_population);
    semaphore_unlock(other_sem_id,HIVE_STRUCTURE_SEM_IDX);
    semaphore_unlock(entry_sem_id,door_num);
}
void stay_in_hive()
{
    sleep(OVERHEAT_TIME);
}

void bee_process(){
setup_bee();
while(entry_count<WORKER_LIFE_TIME){
//TODO pszczoly wychodza i wchodza do ula
stay_in_hive();
leave_hive();
stay_outside();
enter_hive();
}
//TODO: Pszczoła umiera
}
void setup_bee()
{
shm_id = init_shared_hive();
hive = attach_to_hive(shm_id);
hive->bees_population = N;
entry_sem_id = get_entry_semaphores();
other_sem_id = get_other_semaphores();
}
int main(int argc, char *argv[])
{
srand(time(NULL));
if (strcmp(argv[1], "adult") == 0) {
        bee_process(); 
    } else if (strcmp(argv[1], "egg") == 0) {
        incubation();
    } else {
        fprintf(stderr, "Unknown argument: %s\n", argv[1]);
        exit(1);
    }
return 0;
}

