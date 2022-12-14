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
void selectie_optiune(void *, char[20]);
void creare_cont(void *);
void logare(void *);
void optiuni_pentru_utilizator(void *, char[20] );
void alegere_utilizator_mesaj(void *, char[20]);
void inserare_mesaj(void *, char[20]);
void vizualizare_mesaj(void *, char[20]);

static void *treat(void * arg)
{
    struct thData tdL;
    tdL= *((struct thData*)arg);
    char username[20];

    printf ("[thread]- %d - Conectare cu succes...\n", tdL.idThread);
    fflush (stdout);
    pthread_detach(pthread_self());
    selectie_optiune((struct thData*)arg, username);
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

void inserare_msj(void *arg, char username[20], char destinatar[20]){

    struct thData tdL;
    tdL= *((struct thData*)arg);

    char mesaj[1024] = "Scrie un mesaj!\n"
                       "In caz doriti sa parasiti mesageria scrieti"
                       " comanda \"optiuni\" \n";

    char mesaj_nou[1024];
    bzero(mesaj_nou, 1024);

    if(write(tdL.cl, mesaj, 1024) <= 0){
        perror("[server] Eroare la write in inserare msj");
    }

    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;
    char status[20] = "unseen";

    int rc = sqlite3_open("../mess.db", &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
    char iesire[24] = "optiuni";

    while (read(tdL.cl, mesaj_nou, 1024)){

        if (strcmp(mesaj_nou, iesire) == 0){
            optiuni_pentru_utilizator((struct thData *) arg, username);
        }
        else {
            sqlite3_stmt *stmt;

            char *sql = "INSERT INTO mess (username_send, username_recv, message, status) VALUES (?1, ?2, ?3, ?4);";

            sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

            sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, destinatar, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, mesaj_nou, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, status, -1, SQLITE_STATIC);
            sqlite3_step(stmt);
            bzero(mesaj_nou, 1024);

            sqlite3_close(db);
        }
    }
}

void alegere_utilizator_mesaj(void *arg, char username[20]) {
    struct thData tdL;
    tdL = *((struct thData *) arg);
    char comanda[100];
    char utilizator_trimitere[1024] = "Selectati un utilizator caruia doriti"
                                      " sa-i trimite-ti un mesaj\n";
    char nume_utilizator[100];
    bzero(nume_utilizator, 100);

    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;


    int rc = sqlite3_open("../users.db", &db);

    if (rc != SQLITE_OK) {

        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }


    rc = sqlite3_prepare_v2(db, "SELECT * from users WHERE username != ?1;", -1, &res, 0);
    char* nume;
    sqlite3_bind_text(res, 1, username, -1, SQLITE_STATIC);

    while (sqlite3_step(res) != SQLITE_DONE) {

        nume = sqlite3_column_text(res, 0);
        strcat(utilizator_trimitere, nume);
        strcat(utilizator_trimitere, "\n");

    }
    if (write(tdL.cl, utilizator_trimitere, 1024) <= 0) {
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server]Eroare la write in optiuni.\n");
    }
    char aprobare[10] = "Da";

    while (read(tdL.cl, nume_utilizator, 100)) {

        printf("Utilizatorul doreste sa-i trimita msj lui %s\n", nume_utilizator);

        rc = sqlite3_prepare_v2(db, "SELECT * from users WHERE username != ?1;", -1, &res, 0);
        char* utilizator;
        sqlite3_bind_text(res, 1, username, -1, SQLITE_STATIC);

        while (sqlite3_step(res) != SQLITE_DONE) {

            utilizator = sqlite3_column_text(res, 0);

            if(strcmp(utilizator, nume_utilizator) == 0){
                printf("Numele este introdus corect\n");
                write(tdL.cl, aprobare, 10);
                inserare_msj((struct thData*)arg, username,nume_utilizator);
                break;
            }

        }
    }
}
void vizualizare_mesaj(void * arg, char username[20]){
    struct thData tdL;
    tdL= *((struct thData*)arg);

    char mesaje_nevazute[10000];
    char optiune_noua[24];

    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;
    printf("Am ajuns in vizualizare");

    int rc = sqlite3_open("../mess.db", &db);

    if (rc != SQLITE_OK) {

        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }


    rc = sqlite3_prepare_v2(db, "SELECT * from mess WHERE"
                                " username_recv = ?1 and status = ?2;", -1, &res, 0);
    char* nume;
    char* mesaj;
    char status[10] = "unseen";

    sqlite3_bind_text(res, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(res, 2, status, -1, SQLITE_STATIC);

    while (sqlite3_step(res) != SQLITE_DONE) {

        nume = sqlite3_column_text(res, 0);
        mesaj = sqlite3_column_text(res, 2);

        strcat(mesaje_nevazute, "[");
        strcat(mesaje_nevazute, nume);
        strcat(mesaje_nevazute, "]");
        strcat(mesaje_nevazute, mesaj);
        strcat(mesaje_nevazute, "\n");

    }

    if(write(tdL.cl, mesaje_nevazute, 10000)<=0){
        perror("[Server] Eroare la write in mesaje_nevazute");
    }

    rc = sqlite3_prepare_v2(db, "UPDATE mess SET status = ?1"
                                " WHERE username_recv = ?2;", -1, &res, 0);

    char new_status[10] = "seen";

    sqlite3_bind_text(res, 1, new_status, -1, SQLITE_STATIC);
    sqlite3_bind_text(res, 2, username, -1, SQLITE_STATIC);

    while (sqlite3_step(res) != SQLITE_DONE) {

        nume = sqlite3_column_text(res, 0);
        mesaj = sqlite3_column_text(res, 2);

        strcat(mesaje_nevazute, "[");
        strcat(mesaje_nevazute, nume);
        strcat(mesaje_nevazute, "]");
        strcat(mesaje_nevazute, mesaj);
        strcat(mesaje_nevazute, "\n");

    }
    char optiunie[24] = "mesaj";
    while (read(tdL.cl, optiune_noua, 24)){
        if(strcmp(optiune_noua, optiunie) == 0){
            printf("Ati selectat sa trimite-ti mesaj nou \n");
            alegere_utilizator_mesaj((struct thData*)arg, username);
        }
    }
}
void optiuni_pentru_utilizator(void * arg, char username[20]){
    struct thData tdL;
    tdL= *((struct thData*)arg);
    char nr_optiune[10];
    bzero(nr_optiune, 10);
    char optiuni_noi[1024] ="Scrieti numarul corespunzator optiunii dorite \n"
                            "1. Vizualizare mesaje primite \n"
                            "2. Iesire din meniul principal\n"
                            "3. Trimite mesaj\n";

    if (write(tdL.cl, optiuni_noi, 1024) <= 0){
        printf("[Thread %d]\n",tdL.idThread);
        perror ("[server]Eroare la write in optiuni.\n");
    }
    char nr[10] = "3";

    while (read(tdL.cl, nr_optiune, 10) ){
        printf("%s\n", nr_optiune);

        if(strcmp(nr_optiune, "1") == 0){
            printf("[Thread %d]Utilizatorul a ales vizualizare mesaje\n", tdL.idThread);
            vizualizare_mesaj((struct thData*) arg, username);
        }
        else if(strcmp(nr_optiune, "2") == 0){
            printf("[Thread %d]Utilizatorul a ales iesire din meniu secundar\n", tdL.idThread);
            selectie_optiune((struct thData*) arg, username);
        }
        else if(strcmp(nr_optiune, "3") == 0){
            printf("[Thread %d]Utilizatorul a ales trimitere mesaj\n", tdL.idThread);
            alegere_utilizator_mesaj((struct thData*) arg, username);
        }
//        else if(strcmp(nr_optiune, "4") == 0){
//            printf("[Thread %d]Utilizatorul a ales iesire din aplicatie\n", tdL.idThread);
//            bzero(nr_optiune, 10);
//            break;
        else{
            printf("Nu exista optiunea aleasa!\n");
        }
    }
}

void creare_cont(void * arg){
    struct thData tdL;
    tdL= *((struct thData*)arg);

    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;

    int rc = sqlite3_open("../users.db", &db);

    if (rc != SQLITE_OK) {

        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char username[20];
    char password[30];
    char user_command[256] = "..Introduce-ti un nume de utilizator..";
    char password_command[256] = "..Introduce-ti o parola de utilizator..";
    char aver[1] = "1";

    bzero(username, 20);
    bzero(password, 30);

    int ok = 0;

    if(write(tdL.cl, user_command, 256) <= 0 ){
        perror("[server] Eroare la write in user_command");
    }

    if (read(tdL.cl, username, 20) <= 0) {
        perror("[server] Eroare citire username de la client.\n");
    }

    username[strlen(username) - 1] = '\0';


    if(write(tdL.cl, password_command, 256) <= 0 ){
        perror("[server] Eroare la write in user_command");
    }

    if(read(tdL.cl, password, 30) <= 0)
    {
        perror("[server] Eroare citire username de la client.\n");
        exit(1);
    }


    password[strlen(password) - 1] = '\0';

    rc = sqlite3_prepare_v2(db, "SELECT * from users ;", -1, &res, 0);

    char *nume;

    while (sqlite3_step(res) != SQLITE_DONE) {
        nume = sqlite3_column_text(res, 0);

        if (strcmp(username, nume) == 0) {
            printf("User deja existent\n");
            write(tdL.cl, aver, sizeof(char ));
            sqlite3_stmt *stmt;
            int online = 1;
            char *sql = "UPDATE users SET online = ?1 WHERE username = ?2;";

            sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

            sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, online);
            sqlite3_step(stmt);
            creare_cont((struct thData*) arg);
        }
    }

    sqlite3_finalize(res);

    printf("Username: %s\n", username);
    printf("Password: %s\n", password);
    printf("Contul a fost verificat si aprobat \n");

    sqlite3_stmt *stmt;
    int online = 1;

    char *sql = "INSERT INTO users (username, password, online) VALUES (?1, ?2, ?3);";

    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, online);
    sqlite3_step(stmt);
    optiuni_pentru_utilizator((struct thData *) arg, username);

    sqlite3_close(db);
}

void logare(void * arg){
    struct thData tdL;
    tdL= *((struct thData*)arg);

    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;

    int rc = sqlite3_open("../users.db", &db);

    if (rc != SQLITE_OK) {

        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char username[20];
    char password[30];
    char user_command[256] = "..Introduce-ti un nume de utilizator..";
    char password_command[256] = "..Introduce-ti o parola de utilizator..";
    char aver[1] = "1";

    bzero(username, 20);
    bzero(password, 30);


    if(write(tdL.cl, user_command, 256) <= 0 ){
        perror("[server] Eroare la write in user_command");
    }

    if (read(tdL.cl, username, 20) <= 0) {
        perror("[server] Eroare citire username de la client.\n");
    }

    username[strlen(username) - 1] = '\0';


    if(write(tdL.cl, password_command, 256) <= 0 ){
        perror("[server] Eroare la write in user_command");
    }

    if(read(tdL.cl, password, 20) <= 0)
    {
        perror("[server] Eroare citire username de la client.\n");
        exit(1);
    }


    password[strlen(password) - 1] = '\0';

    rc = sqlite3_prepare_v2(db, "SELECT * from users ;", -1, &res, 0);

    char *nume;
    char *parola;
    int ok = 0;

    while (sqlite3_step(res) != SQLITE_DONE) {
        nume = sqlite3_column_text(res, 0);
        parola = sqlite3_column_text(res, 1);
        if (strcmp(username, nume) == 0 && strcmp(password, parola) == 0) {
            printf("User este bun\n");
            write(tdL.cl, aver, 1);

            sqlite3_stmt *stmt;
            int online = 1;

            char *sql = "UPDATE users SET online = ?1 WHERE username = ?2 and password = ?3;";

            sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

            sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 3, online);
            sqlite3_step(stmt);

            optiuni_pentru_utilizator((struct thData *) arg, username);

            sqlite3_close(db);

            optiuni_pentru_utilizator((struct thData *) arg, username);

        }
    }

    sqlite3_finalize(res);
    logare((struct thData *)arg);

    sqlite3_close(db);
}

//void logare(void * arg){
//    struct thData tdL;
//    tdL= *((struct thData*)arg);
//
//    sqlite3 *db;
//    char *err_msg = 0;
//    sqlite3_stmt *res;
//
//    int rc = sqlite3_open("../users.db", &db);
//
//    if (rc != SQLITE_OK) {
//
//        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
//        sqlite3_close(db);
//        exit(1);
//    }
//
//    char username[20];
//    char password[20];
//    char user_command[256] = "....Introduce-ti numele de utilizator....";
//    char password_command[256] = "....Introduce-ti parola de utilizator....";
//
//    bzero(username, 20);
//    bzero(password, 20);
//
//
//    if(write(tdL.cl, user_command, 256) <= 0 ){
//        perror("[server] Eroare la write in user_command");
//    }
//
//    if (read(tdL.cl, username, 20) <= 0) {
//        perror("[server] Eroare citire username de la client.\n");
//        exit(1);
//    }
//    username[strlen(username) - 1] = '\0';
//
//
//    if(write(tdL.cl, password_command, 256) <= 0 ){
//        perror("[server] Eroare la write in user_command");
//    }
//
//    if(read(tdL.cl, password, 20) <= 0)
//    {
//        perror("[server] Eroare citire username de la client.\n");
//        exit(1);
//    }
//
//
//    password[strlen(password) - 1] = '\0';
//
//    rc = sqlite3_prepare_v2(db, "SELECT * from users ;", -1, &res, 0);
//
//    char *nume;
//    char *parola;
//    int ok = 0;
//    char aver[10] = "1";
//    int online = 1;
//
//    while (sqlite3_step(res) != SQLITE_DONE) {
//
//        nume = sqlite3_column_text(res, 0);
//        parola = sqlite3_column_text(res, 1);
//
//        if (strcmp(username, nume) == 0 && strcmp(password, parola) == 0) {
//            printf("Logarea s-a efectuat cu succes\n");
//            write(tdL.cl, aver, 10);
//
//            sqlite3_stmt *stmt;
//
//            char *sql = "INSERT INTO users (online) VALUES (?1) WHERE username= ?2 and password = ?3;";
//
//            sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
//
//            sqlite3_bind_int(stmt, 1, online);
//            sqlite3_bind_text(stmt, 2, username, -1, SQLITE_STATIC);
//            sqlite3_bind_text(stmt, 3, password, -1, SQLITE_STATIC);
//            sqlite3_step(stmt);
//
//            optiuni_pentru_utilizator((struct thData *) arg, username);
//        }
//    }
//
//    sqlite3_finalize(res);
//
//    logare((struct thData *) arg);
//
//    sqlite3_finalize(res);
//    sqlite3_close(db);
//}

void selectie_optiune(void * arg, char username[20]){

    struct thData tdL;
    tdL= *((struct thData*)arg);
    char optiune[15];
    bzero(optiune, 15);

    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;

    int rc = sqlite3_open("../users.db", &db);

    if (rc != SQLITE_OK) {

        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
    char comenzi[1024] = "Scrieti comanda pe care doriti sa o alegeti \n"
                         "1. Logare "
                         "2. Inregistrare "
                         "3. Iesire";
    char avertizare[128] = "Optiunea aleasa nu este buna!";

    if (write(tdL.cl, comenzi, 1024) <= 0){
        printf("[Thread %d] ",tdL.idThread);
        perror ("[Thread]Eroare la write().\n");
    }

    while (read(tdL.cl, optiune, 15) ){
        printf("%s\n", optiune);
        if(strcmp(optiune, "Inregistrare") == 0){
            printf("%s\n", optiune);
            bzero(optiune, 15);
            creare_cont((struct thData*)arg);
        }
        else if(strcmp(optiune, "Logare") == 0){
            printf("[Thread %d]Utilizatorul a ales %s\n", tdL.idThread, optiune);
            bzero(optiune, 15);
            logare((struct thData*)arg);
        }
        else if(strcmp(optiune, "Iesire") == 0){
            printf("[Thread %d]Utilizatorul a ales %s\n", tdL.idThread, optiune);
            bzero(optiune, 15);
            sqlite3_stmt *stmt;
            int online = 0;
            char *sql = "UPDATE users SET online = ?1 WHERE username = ?2;";

            sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

            sqlite3_bind_int(stmt, 1, online);
            sqlite3_bind_text(stmt, 2, username, -1, SQLITE_STATIC);
            sqlite3_step(stmt);
            sleep(10000);
        }
        else {
            printf("Optiunea aleasa nu este buna\n");
            bzero(optiune, 15);
            write(tdL.cl, avertizare, 128);
        }
    }
}