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
void creare_cont(void *);
void optiuni_pentru_utilizator(void *);

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

void optiuni_pentru_utilizator(void * arg){
    struct thData tdL;
    tdL= *((struct thData*)arg);
    char nr_optiune[5];
    char optiuni_noi[1024] = "Salu"
                            "Scrieti numarul corespunzator optiunii dorite \n"
                            "1. Vizualizare mesaje primite \n"
                            "2. Vizualizati oamenii activi in retea\n"
                            "3. Trimite mesaj";

    if (write(tdL.cl, optiuni_noi, 1024) <= 0){
        printf("[Thread %d]\n",tdL.idThread);
        perror ("[server]Eroare la write in optiuni.\n");
    }

    while (read(tdL.cl, nr_optiune, 5) ){
        if(strcmp(nr_optiune, "1") == 0){
            printf("[Thread %d]Utilizatorul a ales vizualizare mesaje\n", tdL.idThread);
        }
        else if(strcmp(nr_optiune, "2") == 0){
            printf("[Thread %d]Utilizatorul a ales vizualizare oameni activi\n", tdL.idThread);
        }
        else if(strcmp(nr_optiune, "3") == 0){
            printf("[Thread %d]Utilizatorul a ales trimitere mesaj\n", tdL.idThread);
        }
    }
}

void creare_cont(void * arg){
    struct thData tdL;
    tdL= *((struct thData*)arg);

    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;

    int rc = sqlite3_open("../test.db", &db);

    if (rc != SQLITE_OK) {

        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char username[20];
    char password[30];
    char user_command[256] = "..Introduce-ti un nume de utilizator..";
    char password_command[256] = "..Introduce-ti o parola de utilizator..";

    bzero(username, 20);
    bzero(password, 30);
    char avertizare[256] = "Userul este deja in baza de date";

    int ok = 0;


    if(write(tdL.cl, user_command, 256) <= 0 ){
        perror("[server] Eroare la write in user_command");
    }

    if (read(tdL.cl, username, 20) <= 0) {
        perror("[server] Eroare citire username de la client.\n");
        exit(1);
    }
    username[strlen(username) - 1] = '\0';

    printf("Username: %s\n", username);


    if(write(tdL.cl, password_command, 256) <= 0 ){
        perror("[server] Eroare la write in user_command");
    }

    if(read(tdL.cl, password, 30) <= 0)
    {
        perror("[server] Eroare citire username de la client.\n");
        exit(1);
    }


    password[strlen(password) - 1] = '\0';

    rc = sqlite3_prepare_v2(db, "SELECT * from mess ;", -1, &res, 0);

    char *nume;
    char aver[4] = "1";
    while (sqlite3_step(res) != SQLITE_DONE) {
        nume = sqlite3_column_text(res, 1);
        printf("%s\n", nume);

        if (strcmp(username, nume) == 0) {
            printf("User deja existent\n");
            write(tdL.cl, aver, 4);
            creare_cont((struct thData*) arg);
        }
    }

    sqlite3_finalize(res);
    sqlite3_close(db);

    printf("Username: %s\n", username);
    printf("Password: %s\n", password);
    printf("Contul a fost verificat si aprobat \n");
    // de verificat cu baza de date pentru maine
    optiuni_pentru_utilizator((struct thData *) arg);
}


void selectie_optiune(void * arg){

    struct thData tdL;
    tdL= *((struct thData*)arg);
    char optiune[15];
    char comenzi[1024] = "Scrieti comanda pe care doriti sa o alegeti \n"
                        "1. Logare "
                        "2. Inregistrare "
                        "3. Iesire";
    char avertizare[128] = "Optiunea aleasa nu este buna!";
//    printf("Am ajuns in verificarea optiunii alese\n");

    if (write(tdL.cl, comenzi, 1024) <= 0){
        printf("[Thread %d] ",tdL.idThread);
        perror ("[Thread]Eroare la write().\n");
    }

    while (read(tdL.cl, optiune, 15) ){
        if(strcmp(optiune, "Inregistrare") == 0){
            printf("%s\n", optiune);
            creare_cont((struct thData*)arg);
        }
        else if(strcmp(optiune, "Logare") == 0){
        printf("[Thread %d]Utilizatorul a ales %s\n", tdL.idThread, optiune);
        } else{
            printf("Optiunea aleasa nu este buna\n");
            write(tdL.cl, avertizare, 128);
        }

    }
//

//
//    else if(strcmp(optiune, "Inregistrare") == 0) {
//        printf("[Thread %d]Utilizatorul a ales %s\n", idThread, optiune);
//        strcpy(validare, "S-a validat optiunea");
//        write(cl, validare, 30);
//        creare_cont(cl, idThread);
//
//    }
}
