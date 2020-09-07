#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define S_MUTEX 0
#define S_FREE_POSITIONS 1 
#define S_ITEM_IN_STACK  2
#define STACK_DIM 10

#define ITEM_PRODUCED 1000
#define ITEM_CONSUMED 1000

#define PRODUCING_SPEED 0.4
#define CONSUMING_SPEED 0.9

void WAIT (short semaphore_id, short semaphore)
{
    struct sembuf operation; 
    operation.sem_num = semaphore; 
    operation.sem_op = -1; 
    operation.sem_flg = 0; 
    semop(semaphore_id, &operation, 1); 
}

void SIGNAL (short semaphore_id, short semaphore)
{
    struct sembuf operation; 
    operation.sem_num = semaphore; 
    operation.sem_op = +1; 
    operation.sem_flg = 0; 
    semop(semaphore_id, &operation, 1); 
}

void producer (short shared_memory_id, short semaphore_id)
{
    int * stack = shmat(shared_memory_id, NULL, 0); 
    short stack_head; 
    short item; 

    if (stack == (int *) -1) {
        perror("[shmat"); 
        exit(1); 
    }

    for (int i = 0; i < ITEM_PRODUCED; i++) {

        item = rand() % 100; 

        WAIT(semaphore_id, S_FREE_POSITIONS); 
        WAIT(semaphore_id, S_MUTEX); 

        stack_head = stack[0] + 1; 
        stack[stack_head] = item;
        stack[0]++; 

        printf("il produttore ha inserito un item (%d) \n", item); 

        SIGNAL(semaphore_id, S_MUTEX); 
        SIGNAL(semaphore_id, S_ITEM_IN_STACK);         

        sleep(PRODUCING_SPEED); 
    }
    exit(0); 
}

void consumer (short shared_memory_id, short semaphore_id)
{
    int * stack = shmat(shared_memory_id, NULL, 0); 
    short stack_head; 
    short item; 

    if (stack == (int *) -1) {
        perror("[shmat"); 
        exit(1); 
    }

    for (int i = 0; i < ITEM_CONSUMED; i++) {

        WAIT(semaphore_id, S_ITEM_IN_STACK);
        WAIT(semaphore_id, S_MUTEX); 

        item = stack[stack[0]]; 
        stack[0]--; 

        printf("il consumatore ha consumato un item (%d) \n", item); 

        SIGNAL(semaphore_id, S_MUTEX);
        SIGNAL(semaphore_id, S_FREE_POSITIONS); 

        sleep(CONSUMING_SPEED); 
    }

    exit(0); 
}

int main(int argc, char * argv[]) 
{
    short shared_memory_id; 
    short semaphores_id; 

    /**
     * Alloco una porzione di memoria pari a STACK_DIM + 1. 
     * La prima posizione conterrà il numero di elementi nello 
     * stack, dalla seconda in poi vi sarà lo stack. 
     */
    const short allocation_size = (STACK_DIM + 1) * sizeof(int);  
    shared_memory_id = shmget(IPC_PRIVATE, allocation_size, IPC_CREAT | 0666);

    if (shared_memory_id < 0) {
        perror("[shmget]"); 
        exit(1); 
    }

    /**
     * Creo 3 semafori con i seguenti scopi: 
     * S_MUTEX: gestire la mutua esclusione.
     * S_EMPTY: conteggio degli elementi all'interno dello stack.
     * S_FULL:  conteggio delle celle libere nello stack.
     */
    semaphores_id = semget(IPC_PRIVATE, 3, IPC_CREAT | 0666); 

    if (semaphores_id < 0) {
        perror("[semget]");
        exit(1);
    }

    /**
     * Inizializzo i semafori al loro valore originale 
     */ 
    semctl(semaphores_id, S_ITEM_IN_STACK, SETVAL, 0); 
    semctl(semaphores_id, S_FREE_POSITIONS, SETVAL, 10); 
    semctl(semaphores_id, S_MUTEX, SETVAL, 1); 

    /**
     * Inizializzo lo stack azzerandolo e pongo valore della 
     * prima cella (contatore di elementi) a 0. 
     */  
    int * stack = shmat(shared_memory_id, NULL, 0); 
    memset(stack, 0, allocation_size); 

    /**
     * Eseguo su un processo figlio il producer 
     */
    if (fork() == 0) {
        producer(shared_memory_id, semaphores_id); 
        perror("[producer fork]"); 
        exit(1);
    }

    /**
     * Eseguo su un processo figlio il consumer  
     */
    if (fork() == 0) {
        consumer(shared_memory_id, semaphores_id); 
        perror("[consumer fork]"); 
        exit(1);
    }

    /**
     * Attendo la fine dell'esecuzione dei due processi
     */ 
    wait(NULL); 
    wait(NULL); 

    /**
     * Distruggo la memoria condivisa e i semafori allocati
     */ 
    shmctl(shared_memory_id, IPC_RMID, NULL);
    semctl(semaphores_id, 0, IPC_RMID, 0);
    exit(0); 
}