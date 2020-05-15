#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

/**
 * Le code di messaggi sono strutture permanenti, 
 * quindi sopravvivono al processo che le ha create.
 * Bisogna cancellarle esplicitamente. Sono
 * permanenti anche i messaggi inseriti e non ancora
 * estratti. 
 * Le code di messaggi sono volatili, ovvero non 
 * sopravvivono al riavvio della macchina. 
 */ 

/**
 * Utilità: utilizzare il comando ipcs per controllare
 * le code allocate, i segmenti di memoria condivisa e
 * i semafori instanziati. Il comando ipcrm è utile per 
 * cancellare manualmente le risorse. 
 */

int main (int argc, char* argv[]) {

    /**
     * Le code di messaggi vengono identificate da
     * da un numero intero positivo, detto chiave. 
     * Diversi processi utilizzano la stessa chiave
     * per riferirsi alla medesima coda.
     * (il tipo key_t è un banale int)  
     * Per scegliere una chiave comune si può 
     * guardare alla funzione ftok().
     * Come chiave key si pu`o utilizzare il valore speciale IPC_PRIVATE per
     * specificare una nuova coda non ancora creata (utilizzabile solo con
     * processi imparentati).
     */

    key_t   queuekey = 50; 

    /**
     * Dal manpage, il valore di ritorno di msgget: 
     * If successful, the return value will be the message queue identifier (a
     * nonnegative integer), otherwise -1 with errno indicating the error.
     */ 

    int qd = 0; 

    printf ("Creo la coda di messaggi di chiave %d\n", queuekey); 

    /**
     * Utilizziamo la chiamata msgget per aprire la coda di 
     * messaggi, passando dei flag: 
     * 
     * IPC_CREAT:   se la coda non esiste, viene creata
     * IPC_EXCL:    se la coda esiste già, termina il programma e
     *              manda un errore
     */ 
    qd = msgget(queuekey, IPC_CREAT | IPC_EXCL | 0660); 

    if (qd == -1) {
        perror("msgget"); 
        exit(1); 
    }

    printf("Coda creata con descrittore %d\n", qd); 

    sleep(3); 

    /**
     *  Per cancellare una coda, modificarne i permessi o raccogliere delle
     *  statistiche su di essa, si pu`o usare la chiamata:
     *  int msgctl(int msqid, int cmd, struct msqid_ds *buf)
     *  Esegue il comando corrispondente a cmd sulla coda di descrittore msqid; le
     *  operazioni specificabili attraverso cmd sono:
     *  
     *  * IPC_RMID: rimuove la coda associata a msqid (buf può essere NULL);
     *  * IPC_STAT: raccoglie alcune statistiche sulla coda e le inserisce nella
     *              struttura puntata da buf (non scendiamo nei dettagli);
     *  * IPC_SET:  reimposta i diritti di accesso alla coda secondo quanto
     *              specificato nella struttura puntata da buf (non scendiamo nei
     *              dettagli).
     */

    printf("Chiudo la coda\n"); 
    msgctl(qd, IPC_RMID, NULL); 
}