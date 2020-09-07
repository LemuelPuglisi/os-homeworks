#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <wait.h>
#include <string.h>
#include <semaphore.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>

/*******************************************************************
 * Implementazione della soluzione del problema producer-consumer  *
 * attraverso l'utilizzo dei semafori.                             *
 *******************************************************************/ 

#define STACK_SIZE 10

/* identificatori dei semafori */
/* nell'array di semafori      */

#define MUTEX   0
#define FULL    1
#define EMPTY   2

/**
 * Implementiamo le azioni di SIGNAL e WAIT viste nei testi
 * attraverso le chiamate di sistema fornite 
 */ 

/**
 * È necessario parlare della chiamata semop: 
 * Essa prende in input tre argomenti: 
 * 
 * @param int semid         | l'identificativo del semaforo
 * @param struct sembuf *op | un'array di struct che contiene: 
 *          @param int semnum   | il numero di semaforo su cui eseguire l'operazione
 *          @param int oper     | l'operazione da svolgere [signal: >0] [wait: <0]
 *          @param int flags    | alcuni flag applicabili, come IPC_NOWAIT 
 * @param int nops          | il numero di operazioni (quindi struct sembuf) da effettuare. 
 * 
 * Passando più struct sembuf, verranno eseguite più operazioni sui semafori in maniera
 * atomica.  
 */

void UP (int sem_id, int sem_number, short increment) {

    if (increment < 0) {
        printf("cannot increment the semaphore with a non-positive number.\n"); 
        return; 
    }

    struct sembuf signal = {sem_number, increment, 0}; 
    int cmd = semop(sem_id, &signal, 1); 

    if (cmd == -1) {
        perror("error in signal execution \n"); 
        exit(1); 
    }
}


void DOWN (int sem_id, int sem_number) {

    struct sembuf wait = {sem_number, -1, 0}; 
    int cmd = semop(sem_id, &wait, 1); 
    
    if (cmd == -1) {
        perror("error in wait execution \n"); 
        exit(1); 
    }
}

void producer (short semaphore_id, short shared_memory_id) 
{
    printf("producer avviato.\n");
    short item; 
    short head; 
    int * stack = shmat(shared_memory_id, NULL, 0); 

    if (stack == (void *) -1) {
        perror("produce shmat"); 
        exit(1); 
    }

    while (1) {

        item = rand() % 100; // produce un elemento
        DOWN(semaphore_id, EMPTY); 
        DOWN(semaphore_id, MUTEX); 

        head = stack[0];                // controlla quanti elementi vi sono nello stack
        stack[1 + head + 1] = item;     // inserisce l'elemento nel primo posto disponibile 
        stack[0]++;                     // incrementa il numero di elementi nello stack
        printf("il produttore ha inserito %d nello stack.", item); 

        UP(semaphore_id, MUTEX, 1); 
        UP(semaphore_id, FULL, 1); 
        sleep(1);
    }
}

void consumer (short semaphore_id, short shared_memory_id)
{
    printf("consumer avviato.\n");
    short item; 
    short head; 
    int * stack = shmat(shared_memory_id, NULL, 0); 

    while (1) {
        DOWN(semaphore_id, FULL); 
        DOWN(semaphore_id, MUTEX); 

        head = stack[0];            // controlla quanti elementi vi sono nello stack
        item = stack[1 + head];     // legge l'ultimo elemento 
        stack[0]--;                 // decrementa il numero di elementi nello stack
        
        printf("il consumatore ha rimosso %d dallo stack.", item); 

        UP(semaphore_id, MUTEX, 1); 
        UP(semaphore_id, EMPTY, 1); 
        sleep(1); 
    }
} 

int main () {

    /**
     * Allochiamo un segmento di memoria condiviso che utilizzeremo
     * come stack. Il primo elemento della memoria condivisa indicherà 
     * la quantità di elementi nello stack.  
     */

    const size_t    sh_mem_size = (1 + STACK_SIZE) * sizeof(int);  
    const int       sh_mem_mask = IPC_CREAT | 0600; 

    /* alloco la memoria condivisa */
    const int sh_mem_id = shmget(IPC_PRIVATE, sh_mem_size, sh_mem_mask); 

    if (sh_mem_id == -1) {
        perror("error in shared memory allocation \n"); 
        exit(1); 
    }

    /**
     * L'accesso concorrente allo stack verrà gestito attraverso 
     * 3 semafori: MUTEX, EMPTY e FULL. Provvediamo all'allocazione:  
     */ 

    const int sem_mask = sh_mem_mask; 

    /**
     * Viene allocato un array di 3 semafori, identificato
     * dall'id ritornato dalla funzione. 
     */

    const int sem_id = semget(IPC_PRIVATE, 3, sem_mask); 

    if (sem_id == -1) {
        perror("error in semaphore allocation \n");
        exit(1); 
    }

    /**
     * Inizializziamo i semafori 
     * - EMPTY: il numero di celle libere all'interno dello stack
     * - FULL:  il numero di elementi all'interno dello stack
     * - MUTEX: il semaforo per la mutua esclusione   
     */     

    UP(sem_id, EMPTY, STACK_SIZE); 
    UP(sem_id, FULL,  0); 
    UP(sem_id, MUTEX, 1); 

    /**
     * Ripuliamo la memoria condivisa inizializzandola a 0. 
     */ 
    int * stack = shmat(sh_mem_id, NULL, 0); 
    memset(stack, 0, sh_mem_size); 

    if (fork() == 0) {
        consumer(sem_id, sh_mem_id); 
        perror("[fork consumer]"); 
        exit(1);
    }

    if (fork() == 0) {
        producer(sem_id, sh_mem_id); 
        perror("[fork producer]"); 
        exit(1);
    }
    
    /** attendo che entrambi concludano **/
    wait(NULL);
    wait(NULL);

    /* rimuovo la memoria condivisa */
    shmctl(sh_mem_id, IPC_RMID, NULL); 

    /* rimuovo i semafori allocati */
    semctl(sem_id, 0, IPC_RMID, 0); 
}