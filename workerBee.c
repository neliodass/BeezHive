#include "hive_manager.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include<string.h>
#include <signal.h>
#define WORKER_LIFE_TIME 4 //Czas życia jednej pszczoły(liczba odwiedzin w ulu)
#define MIN_OVERHEAT_TIME 10
#define MAX_OVERHEAT_TIME 50
 //Czas zanim pszczoła się przegrzeje
#define MIN_INCUB_TIME 5
#define MAX_INCUB_TIME 10
#define MAX_OUT_TIME 15
#define MIN_OUT_TIME 5
bool in_hive=true;
int entry_count = 0;
Hive* hive;
int shm_id;
int entry_sem_id,other_sem_id;
char log_msg[100]; 


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
//if ((hive->bees_in_hive)<(hive->bees_population/2)) semaphore_unlock(entry_sem_id,ENTER_HIVE_IDX);

snprintf(log_msg,sizeof(log_msg),"Bee:%d left the hive. Bees in hive:%d/%d\n",getpid(),hive->bees_in_hive,hive->current_bee_population);
log_message(log_msg);
semaphore_unlock(other_sem_id,HIVE_STRUCTURE_SEM_IDX);
semaphore_unlock(entry_sem_id,door_num);

}
void stay_outside()
{
    int outside_time = rand()%(MAX_OUT_TIME-MIN_OUT_TIME+1)+MIN_OUT_TIME;
    usleep(outside_time*MICROSECOND);
}
void enter_hive()
{
    int door_num = random_door();
    //semaphore_lock(entry_sem_id,ENTER_HIVE_IDX);
    semaphore_lock(entry_sem_id,door_num);
    semaphore_lock(other_sem_id,HIVE_STRUCTURE_SEM_IDX);
    if (hive->bees_in_hive < hive->max_bees_population / 2) {
    hive->bees_in_hive+=1;
    in_hive = true;
    entry_count+=1;
    snprintf(log_msg,sizeof(log_msg),"Bee:%d entered the hive. Bees in hive:%d/%d\n",getpid(),hive->bees_in_hive,hive->current_bee_population);
    log_message(log_msg);
    }
    else{
        snprintf(log_msg,sizeof(log_msg),"Bee:%d could not enter the hive. Hive is full\n",getpid());
        log_message(log_msg);
    }
    semaphore_unlock(other_sem_id,HIVE_STRUCTURE_SEM_IDX);
    semaphore_unlock(entry_sem_id,door_num);
}
void stay_in_hive()
{
    int overheat_time = rand()%(MAX_OVERHEAT_TIME-MIN_OVERHEAT_TIME+1)+MIN_OVERHEAT_TIME;
    usleep(overheat_time*MICROSECOND);
}

void bee_die() {
    semaphore_lock(other_sem_id, HIVE_STRUCTURE_SEM_IDX);
    hive->bees_in_hive -= 1;
    hive->current_bee_population -= 1;
    snprintf(log_msg,sizeof(log_msg),"Bee:%d is dying. Bees in hive:%d/%d\n", getpid(), hive->bees_in_hive, hive->current_bee_population);
    log_message(log_msg);
    semaphore_unlock(other_sem_id, HIVE_STRUCTURE_SEM_IDX);
    detach_from_hive(hive);
    exit(0);
    printf("BEE DIED INCORRECTLY\n");
}
void setup_bee()
{

shm_id = init_shared_hive();
hive = attach_to_hive(shm_id);
entry_sem_id = get_entry_semaphores();
other_sem_id = get_other_semaphores();
}
void bee_process(){
setup_bee();
while(entry_count<WORKER_LIFE_TIME){
if (in_hive)
{
stay_in_hive();
leave_hive();
}
else
{
    stay_outside();
    enter_hive();
}
}

bee_die();
}

void incubation()
{
    int incubation_time = rand() % (MAX_INCUB_TIME - MIN_INCUB_TIME + 1) + MIN_INCUB_TIME;
    usleep(incubation_time*MICROSECOND);
    snprintf(log_msg,sizeof(log_msg),"Bee:%d hatched\n", getpid());
    log_message(log_msg);
    bee_process();
}
void cleanup(int signum)
{
    detach_from_hive(hive);
    snprintf(log_msg,sizeof(log_msg),"Bee:%d cleaned up resources and is exiting.\n", getpid());
    //log_message(log_msg);
    exit(0);
}
int main(int argc, char *argv[])
{
setpgid(0, getppid());
signal(SIGINT, cleanup);
srand(time(NULL)^ getpid());
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

