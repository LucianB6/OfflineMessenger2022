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

void creare_cont(int sd){
    char username[20];
    char password[20];
    char validare_user[20];

    printf("Va rugam sa selectati un username\n");
    bzero(username, 20);

    scanf("%29s", username);
    printf("%s\n", username);
    strtok(username, "\n");

    if(write(sd, username, 20) <= 0){
        perror("[client]Eroare la read in password");
    }

    printf("Va rugam sa selectati o parola\n");
    bzero(password, 20);
    read(0, password, sizeof(password));

    if(write(sd, password, 100) <= 0){
        perror("[client]Eroare la read in password");
    }
}

int main (int argc, char *argv[]) {
    int sd;
    struct sockaddr_in server;    // structura folosita pentru conectare
    char optiune[15];
    char pass[256];
    int val;
    int contor;
    char validare[100];
    char selectie[15];
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
    printf("Scrieti comanda pe care doriti s-o accesati \n");
    printf("1. Logare  ");
    printf("2. Inregistrare  ");
    printf("3. Iesire  \n");

    bzero(optiune, 15);
    read(0, optiune, sizeof(optiune));
    strtok(optiune, "\n");

    if (write(sd, optiune, 100) <= 0) {
        perror("[client] Eroare la write din optiune");
    }
    if (strcmp(optiune, "Inregistrare") <= 0) {
//            creare_cont(sd);
        if (read(sd, &validare, 100) <= 0) {
            perror("[client]Eroare la read in validare");
            }
        printf("%s \n", validare);
//        creare_cont(sd);

        char username[20];
        char password[20];
        char validare_user[20];

        printf("Va rugam sa selectati un username\n");

        bzero(username, 20);
        strtok(username, "\n");
        read(0, username, 20);

        if(write(sd, username, 20) <= 0){
            perror("[client]Eroare la read in password");
        }



        }

    }
//
// Created by lucian on 23.11.2022.
//
