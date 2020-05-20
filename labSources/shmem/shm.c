#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>  

/*
 * Un altro costrutto che UNIX ci mette a disposizione per la comunicazione
 * tra processi è la memoria condivisa. Una memoria condivisa è una porzione
 * di memoria accessibile da più processi.
 * I processi che condividono la stessa area di memoria possono utilizzare tale
 * zona per scambiarsi dei dati in modo arbitrario (leggendovi e scrivendovi).
 * Ogni memoria condivisa ha un nome univoco detto chiave di utilizzo del
 * tutto analogo a quanto visto per le code di messaggi. La chiave è ciò che
 * ogni processo utilizza per riferirsi alla medesima area di memoria da
 * condividere.
 */

#define MAX_NUMBERS 10

void producer (int sh_mem_d);
void consumer (int sh_mem_d);  

int main (int argc, char* argv[]) { 

    /*
     * Se un processo vuole usare un’area di memoria condivisa deve prima aprirla
     * (così come per le code di messaggi) ed attaccarla al proprio spazio di
     * indirizzamento. Una volta fatta questa operazione, può leggervi e scrivervi
     * come una qualunque altra area disponibile, utilizzando le strutture dati più
     * idonee.
     * Quando un processo non desidera più utilizzare quell’area di memoria
     * condivisa può staccarla dal suo spazio di indirizzamento (ciò viene
     * comunque fatto in fase di distruzione del processo). Questa operazione
     * non implica la distruzione dell’area di memoria, infatti (così come per le
     * code di messaggi) si tratta di strutture di memoria permanenti. E’
     * necessario distruggere esplicitamente l’area di memoria (così come per le
     * code di messaggi). Ovviamente, la proprietà di permanenza vale anche per
     * i dati memorizzati nella memoria condivisa.
     */

    const   key_t   sh_mem_key  = IPC_PRIVATE;
    const   size_t  sh_mem_size = sizeof(int) * (MAX_NUMBERS + 1); 
    const   int     sh_mem_perm = IPC_CREAT | 0600;  
            int     sh_mem_desc;

    /**
     * @param key_t  sm_mem_key     | la chiave assegnata alla porzione di memoria condivisa
     * @param size_t sh_mem_size    | specifichiamo la dimensione di 31 interi 
     * @param int    sh_mem_perm    | analogo alla gestione delle code di messaggi
     */   

    sh_mem_desc = shmget(sh_mem_key, sh_mem_size, sh_mem_perm); 

    if (sh_mem_desc == -1) {
        perror("error while creating shared memory"); 
        exit(0); 
    }


    int pid = 0; 

    /**
     * Creazione di un processo figlio che permetta all'utente
     * di inserire dei numeri da tastiera. Essi verranno posizionati
     * in maniera contigua nella memoria condivisa allocata. 
     */ 

    if ((pid = fork()) == 0) {
        producer(sh_mem_desc); 
    }
    else if (pid > 0) {
        wait(NULL); 
    }
    else {
        perror("error while forking the process");
        exit(1); 
    }

    /**
     * Creazione di un processo figlio che legga (consumer) i
     * numeri inseriti precedentemente nella memoria condivisa.
     */

    if ((pid = fork()) == 0) {
        consumer(sh_mem_desc); 
    }
    else if (pid > 0) {
        wait(NULL); 
    }
    else {
        perror("error while forking the process");
        exit(1); 
    }

    /* eliminiamo la memoria condivisa */
    shmctl(sh_mem_desc, IPC_RMID, NULL); 
}

void producer (int sh_mem_d) {

    int c = 0;  

    /**
     * Un processo che voglia sfruttare la memoria condivisa 
     * deve mapparla sul proprio spazio di indirizzamento. 
     * Questa operazione viene effettuata tramite la chiamata 
     * shmat che alloca la memoria virtuale in maniera *contigua*. 
     */

    int * sh_mem_init = NULL;

    /**
     * @param int   shmemid     | descrittore della memoria condivisa allocata
     * @param void* shmemaddr   | indirizzo (facoltativo) di memoria su cui si vuole mappare 
     * @param int   flags       | maschera di flag applicabili alla chiamata di sistema
     * @return void *           | primo indirizzo di memoria della mappatura  
     */ 

    if ((sh_mem_init = shmat(sh_mem_d, NULL, 0)) == (void *) -1) {
        perror("producer shared memory mapping");
        exit(1); 
    }

    int number; 

    do {

        printf("[producer] Insert a number (negative to stop)\n");
        scanf("%d", &number);

        if (number <= 0)
            break; 


        /* inseriamo a partire da 1 */
        sh_mem_init[++c] = number; 

    } while (c < MAX_NUMBERS && number > 0); 

    /**
     * Lasciamo il primo elemento nella memoria libero 
     * così da poter indicare al processo figlio
     * quanti elementi sono stati inseriti. 
     */ 

    sh_mem_init[0] = c; 
    
    exit(0); 
}


void consumer (int sh_mem_d) {

    int     numbers;  

    /**
     * Andando a dichiarare il puntatore della memoria
     * ad (int *) anziché (void *) possiamo trattare 
     * il puntatore ritornato dalla chiamata shmat direttamente
     * come array di interi. 
     */
    
    int *   sh_mem_init = NULL;

    if ((sh_mem_init = shmat(sh_mem_d, NULL, 0)) == (void *) -1) {
        perror("consumer shared memory mapping");
        exit(1); 
    }

    /**
     * Ricordiamo che la prima posizione (0) è occupata 
     * dal numero di elementi inseriti dall'utente
     */

    for (int i = 0; i < sh_mem_init[0]; ++i) 
        printf("[consumer] consuming number %d \n", sh_mem_init[i+1]); 
    
    exit(0); 
}