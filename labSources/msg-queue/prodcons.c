#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

/****************************/
/* PRODUTTORE - CONSUMATORE */ 
/****************************/ 

/**
 * Il programma prendera in input un parametro che 
 * stabilirà se il processo sarà un producer (P) o
 * un consumer. Questi ultimi sono di due tipi: A e B.
 * I consumer di tipo A prelevano dalla coda solo messaggi
 * di tipo A, analogamente i consumer di tipo B con i 
 * messaggi di tipo B. 
 * Il producer immetterà nella coda di messaggi n
 * messaggi di tipo random tra A e B. 
 * Un consumer si ferma alla ricezione di un messaggio 
 * di valore negativo. 
 */

#define MSG_A 1
#define MSG_B 2 
#define Q_KEY 55

#define NMESSAGES 20

void consumer(int queuedesc); 
void producer(int queuedesc, int msgtype); 

/**
 * Le code di messaggi si servono di una struct
 * per comunicare. Nella struct è necessario 
 * definire come primo elemento un long type 
 * che definisce il tipo del messaggio. Questo
 * meccanismo permette ai processi lettori di
 * selezionare il tipo di messaggi da leggere. 
 */

typedef struct {
    long type; 
    int  load; 
} qmsg; 


int main (int argc, char* argv[]) {

    const key_t qkey = Q_KEY; 

    if (argc != 2) {
        printf("usage: %s [P|A|B]\n", argv[0]); 
        exit(1); 
    }

    char procmode = argv[1][0]; 

    int queuedesc = msgget(qkey, IPC_CREAT | 0600); 

    if (queuedesc == -1) {
        printf("error in message queue creation"); 
        exit(1); 
    }

    if (procmode == 'P') {
        consumer(queuedesc); 
    }
    else if (procmode == 'A' || procmode == 'B') {
        producer(queuedesc, procmode == 'A' ? MSG_A : MSG_B); 
    }
    else {
        printf("usage: %s [P|A|B]\n", argv[0]); 
        exit(1); 
    }

    exit(0); 
}

/**
 * con la modalità IPC_NOWAIT su msgsnd e msgrcv la chiamata ritorna subito 
 * un valore -1 (così come in caso di errore) nel caso di coda piena.
 */


void consumer (int queuedesc) {
    
    char types[3] = {'X', 'A', 'B'}; 
    char msgtype; 

    int cmdr = 0, msgcounter = 0; 
    qmsg message; 

    for (int i = 0; i < NMESSAGES; ++i) {

        message.type = 1 +  (rand() % 2); 
        message.load = rand() % 300;

        /**
         * Mando alla coda di messaggi il messaggio generato
         * casualmente. 
         * [ATTENZIONE] non va passato il puntatore alla struct, 
         * altrumenti verrà inserito l'indirizzo di memoria della
         * struct. 
         * [ATTENZIONE] la dimensione passata alla funzione esclude
         * i 4 byte del tipo del messaggio. Una alternativa sarebbe
         * stata passare sizeof(message) - 4.
         */

        cmdr = msgsnd(queuedesc, &message, sizeof(message.load), 0); 
        sleep(2); // usefull to see the flow 
        msgcounter ++; 

        if (cmdr == -1) {
            printf("error in message sending\n"); 
            break; 
        }

        msgtype = message.type == 1 ? 'A' : 'B'; 
        printf("[message n.%d - type: %c - load: %d]\n", msgcounter, msgtype, message.load); 
    }

    /**
     * Terminazione della comunicazione
     * notifichiamo ai producer di tipo A e B che la 
     * comunicazione è terminata con dei load negativi. 
     */
    
    qmsg terminate; 
    terminate.load = -1; 

    terminate.type = MSG_A; 
    cmdr = msgsnd(queuedesc, &terminate, sizeof(message.load), 0); 
    
    terminate.type = MSG_B; 
    cmdr = msgsnd(queuedesc, &terminate, sizeof(message.load), 0); 

    printf("Press a key to delete the message queue\n"); 
    char tmp; fgetc(stdin); 

    printf("starting the remove queue process\n"); 
    msgctl(queuedesc, IPC_RMID, NULL); 
}


void producer(int queuedesc, int msgtype) {

    qmsg message; 
    int cmdr, msgcounter = 0; 
    char cmsgtype = msgtype == 1 ? 'A' : 'B'; 

    do {

        /* Il parametro mtype serve ad estrarre messaggi di un certo tipo:
         * * 0:     viene estratto il primo messaggio disponibile (a prescindere dal tipo)
         *          secondo l’ordine FIFO;
         * * > 0:   viene estratto il primo messaggio disponibile di tipo corrispondente a
         *          mtype;
         * * < 0:   viene estratto il primo tra i messaggi in coda di tipo t, con t minimo e
         *          con t≤|mtype|.
         * 
         * Ritorna -1 in caso di errore o il numero di byte del messaggio letto.
         */

        cmdr = msgrcv(queuedesc, &message, sizeof(int), msgtype, 0);
        msgcounter ++; 

        if (cmdr == -1 || message.type != msgtype) {
            printf("error in message retrieving\n"); 
            exit(0); 
        }

        printf("[message n.%d - type: %c - load: %d]\n", msgcounter, cmsgtype, message.load);
    
    } while (message.load >= 0); 

    exit(0);     
}
