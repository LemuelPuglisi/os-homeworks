#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

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

void SIGNAL (int sem_id, int sem_number) {

    struct sembuf signal = {sem_number, +1, 0}; 
    int cmd = semop(sem_id, &signal, 1); 

    if (cmd == -1) {
        perror("error in signal execution \n"); 
        exit(1); 
    }
}


void WAIT (int sem_id, int sem_number) {

    struct sembuf wait = {sem_number, -1, 0}; 
    int cmd = semop(sem_id, &wait, 1); 
    
    if (cmd == -1) {
        perror("error in wait execution \n"); 
        exit(1); 
    }
}

/**
 * @todo implementare producer 
 */

void producer (); 

/**
 * @todo implementare consumer  
 */

void consumer (); 

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

    /** avvio il producer **/
    if (fork != 0) {

        /** avvio il consumer **/
        if (fork != 0) {

            /** attendo che entrambi concludano **/
            wait(NULL);
            wait(NULL);
        }
        else {
            consumer(); 
        }
    }
    else {
        producer(); 
    }

    /* rimuovo la memoria condivisa */
    shmctl(sh_mem_id, IPC_RMID, NULL); 

    /* rimuovo i semafori allocati */
    semctl(sem_id, 0, IPC_RMID, 0); 
}