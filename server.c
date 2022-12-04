#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sqlite3.h>
#include <stdbool.h>

/* portul folosit */

#define PORT 2741

extern int errno;		/* eroarea returnata de unele apeluri */


typedef struct thData{
    int idThread;
    int cl;
}thData;

int i = 0;
void selectie_optiune(void *);
void creare_cont(struct thData* arg);

static void *treat(void * arg)
{
    struct thData tdL;
    tdL= *((struct thData*)arg);

    printf ("[thread]- %d - Conectare cu succes...\n", tdL.idThread);
    fflush (stdout);
    pthread_detach(pthread_self());
    selectie_optiune((struct thData*)arg);
    /* am terminat cu acest client, inchidem conexiunea */
    return(NULL);
}

/* programul */
int main ()
{
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;
    //int nr;		//mesajul primit de trimis la client
    int sd;		//descriptorul de socket
    int pid;
    pthread_t th[100];    //Identificatorii thread-urilor care se vor crea


    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[server]Eroare la socket().\n");
        return errno;
    }
    /* utilizarea optiunii SO_REUSEADDR */
    int on=1;
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);

    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
        perror ("[server]Eroare la bind().\n");
        return errno;
    }

    if (listen (sd, 2) == -1)
    {
        perror ("[server]Eroare la listen().\n");
        return errno;
    }
    while (1)
    {
        int client;
        int length = sizeof (from);
        thData *td;

        printf ("[server]Asteptam la portul %d...\n",PORT);
        fflush (stdout);

        if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
        {
            perror ("[server]Eroare la accept().\n");
            continue;
        }
        // cream thread-ul
        td=(struct thData*)malloc(sizeof(struct thData));
        td->idThread=i++;
        td->cl=client;

        pthread_create(&th[i], NULL, &treat, td);

    }

}
//
//void creare_cont(struct thData* arg){
//    char username[20];
//    char read_gol[20];
//    char password[20];
//    char validare_user[20];
//    int ok = 0;
//    struct thData tdL;
//
//    tdL = *((struct thData*)arg);
//
//    printf("Am ajuns in creare cont\n");
//
//    if (read(arg->cl, username, 20) < -1) {
//        perror("[server] Eroare la read in username");
//    }
//    printf("Username %s\n", username);
//    if (write(tdL.cl, username, 20) <= 0) {
//        perror("[server] Eroare la read de user");
//    }
//
//    if (read(tdL.cl, password, 20) <= 0) {
//        printf("[Thread %d] ", tdL.idThread);
//        perror("[Thread]Eroare la write().\n");
//    }
//    printf("PArola: %s", password);
//
//
//}

void selectie_optiune(void * arg){

    struct thData tdL;
    char optiune[15];
    char username[20];
    char validare[30] = "";

    tdL = *((struct thData*)arg);

    printf("AM ajuns aici\n");

    if (read(tdL.cl, optiune, 15) <= 0){
        printf("[Thread %d] ",tdL.idThread);
        perror ("[Thread]Eroare la write().\n");
    }


    if(strcmp(optiune, "Logare") == 0){
        printf("[Thread %d]Utilizatorul a ales %s\n", tdL.idThread, optiune);
    }

    else if(strcmp(optiune, "Inregistrare") == 0) {
        printf("[Thread %d]Utilizatorul a ales %s\n", tdL.idThread, optiune);
//
        strcpy(validare, "S-a validat optiunea");
        write(tdL.cl, validare, 30);
        printf("%d\n", tdL.cl);
//        creare_cont(&tdL);

        if (read(tdL.cl, username, 20) <= 0) {
            printf("[Thread %d] ", tdL.idThread);
            perror("[Thread]Eroare la write().\n");
        }
        if(strcmp(username, "")!= 0){
            printf("%s", username);
        }

    }
}
