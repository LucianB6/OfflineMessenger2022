#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;
//

void inserare_mesaj(int sd){
    printf("Am ajuns\n");
}

void trimitere_mesaj(int sd) {
    char utilizator_trimitere[1024];
    char utilizator[1024];
    char aprobare[10];

    if (read(sd, utilizator_trimitere, 1024) <= 0) {
        perror("[client] Eroare la read in optiuni");
    }
    printf("%s \n", utilizator_trimitere);
    char confirmare[10] = "Da";

    while (read(0, utilizator, 100)) {
        utilizator[strlen(utilizator) - 1] = '\0';

        fflush(stdout);
        write(sd, utilizator, 100);

        if (read(sd, aprobare, 10) <= 0) {
            perror("Eroare la read");
        }
        confirmare[strlen(confirmare) - 1] = '\0';
        printf("%s", confirmare);
        if (strcmp(confirmare, aprobare) == 0) {
            printf("A-ti inserat numele corect");
        }
    }
}

void optiuni_pentru_utilizator(int sd) {
    char optiuni[1024];
    char optiune_dorita[5];

    if (read(sd, optiuni, 1024) <= 0) {
        perror("[client] Eroare la read in optiuni");
    }
    printf("%s \n", optiuni);

    while (read(0, optiune_dorita, 5)) {
        optiune_dorita[strlen(optiune_dorita) - 1] = '\0';

        fflush(stdout);
        write(sd, optiune_dorita, 5);

        if (strcmp(optiune_dorita, "1") == 0) {
            printf("Ati selectat %s: vizualizare mesaje \n", optiune_dorita);
        }
        else if (strcmp(optiune_dorita, "2") == 0) {
            printf("Ati selectat %s: vizualizare oameni activi \n", optiune_dorita);
        }
        else if (strcmp(optiune_dorita, "3") == 0) {
            printf("Ati selectat %s: trimitere mesaj \n", optiune_dorita);
            trimitere_mesaj(sd);
        }
        else if (strcmp(optiune_dorita, "4") == 0){
            printf("Ati selectat %s: iesire din aplicatie\n", optiune_dorita);
            exit(1);
        } else printf("Optiunea nu este disponibila!\n");
    }
}
void creare_cont(int sd){
    char username[20];
    char password[20];
    char user_command[256];
    char password_command[256];

    if(read(sd, user_command, 256) <= 0){
        perror("[client]Eroare la read in user_command");
    }
    printf("%s\n", user_command);
    char avertizareClient[256] = "Userul este deja in baza de date";
    char avertizare[1];
    bzero(username, 20);
    read(0, username, 20);

    fflush(stdout);
    int ok = 0;
    if (write(sd, username, 20) <= 0) {
        perror("[client] Eroare la write in username");
    }


    if(read(sd, password_command, 256) <= 0){
        perror("[client]Eroare la read in user_command");
    }
    printf("%s\n", password_command);

    bzero(password, 20);
    read(0, password, 20);

    fflush(stdout);

    if(write(sd, password, 20) <= 0)
    {
        perror("[client]Eroare la scriere spre server.\n");
        return;
    }

    if( read(sd, avertizare, 1) <= 0){
        perror("[client] Eroare la read in avertizare\n");
    }

    if(strcmp(avertizare,"1") == 0){
        printf("Nume de utilizator indisponibil \n");
        creare_cont(sd);
    } else {
        printf("S");
        optiuni_pentru_utilizator(sd);
    }
}

void logare(int sd){
    char username[20];
    char password[20];
    char user_command[256];
    char password_command[256];

    if(read(sd, user_command, 256) <= 0){
        perror("[client]Eroare la read in user_command");
    }
    printf("%s\n", user_command);
    char avertizare[10];

    bzero(username, 20);
    read(0, username, 20);

    fflush(stdout);
    int ok = 0;
    if (write(sd, username, 20) <= 0) {
        perror("[client] Eroare la write in username");
    }


    if(read(sd, password_command, 256) <= 0){
        perror("[client]Eroare la read in user_command");
    }
    printf("%s\n", password_command);

    bzero(password, 20);
    read(0, password, 20);

    fflush(stdout);

    if(write(sd, password, 20) <= 0)
    {
        perror("[client]Eroare la scriere spre server.\n");
        return;
    }

    if(read(sd, avertizare, 10) <= 0){
        perror("[client] Eroare la read in avertizare\n");
    }

    if(strcmp(avertizare, "1") == 0){
        printf("Logarea s-a efectuat cu succes \n");
        optiuni_pentru_utilizator(sd);
    } else {
        logare(sd);
    }
}

int main (int argc, char *argv[]) {
    int sd;
    struct sockaddr_in server;    // structura folosita pentru conectare
    char optiune[1024];
    char selectie[15];
    char avertizare[128];
    /* exista toate argumentele in linia de comanda? */
    if (argc != 3) {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    /* stabilim portul */
    port = atoi(argv[2]);

    /* cream socketul */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Eroare la socket().\n");
        return errno;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

    if (connect(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        perror("[client]Eroare la connect().\n");
        return errno;
    }

    if (read(sd, optiune, 1024) < 0) {
        perror("[client] Eroare la write din optiune");
    }
    printf("%s\n", optiune);

    while (read(0, selectie, 15)) {
        selectie[strlen(selectie) - 1] = '\0';

        fflush(stdout);
        write(sd, selectie, 15);

        if (strcmp(selectie, "Inregistrare") == 0) {
            printf("Ati selectat %s \n", selectie);
            creare_cont(sd);
        }
        else if (strcmp(selectie, "Logare") == 0) {
            printf("Ati selectat %s \n", selectie);
            logare(sd);
        }
        else {
            read(sd, avertizare, 128);
            printf("%s\n", avertizare);
        }
    }

    }
//
// Created by lucian on 23.11.2022.
//
