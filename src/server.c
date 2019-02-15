#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#include "server.h"

#define BUFFSIZE 2048


int main() {

    /* declaration variables */

    int s, c; /* descripteur de socket s = server et c = client */
    struct sockaddr_in6 sin6_server;
    struct sockaddr_in6 sin6_client;
    int port = 8080;




    /* Creation d’un socket serveur */

    s = socket(PF_INET6, SOCK_STREAM, 0);

    if (s < 0) {
        fprintf(stderr, "Erreur socket\n");
        exit(EXIT_FAILURE);
    }


    /* Association d’une adresse sur un port */

    memset(&sin6_server, 0, sizeof(sin6_server));

    sin6_server.sin6_family = PF_INET6;
    sin6_server.sin6_port = htons(port);

    


    /* permettre la reutilisation du port */

    int val = 1;
    int rc_setsocket = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    if (rc_setsocket < 0) {
        fprintf(stderr, "Erreur setsocket\n");
        exit(EXIT_FAILURE);
    }
    //polymorphisme
    val = 0;
    rc_setsocket = setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof(val));

    if (rc_setsocket < 0) {
        fprintf(stderr, "Erreur setsocket\n");
        exit(EXIT_FAILURE);
    }


    int rc_bind = bind(s, (struct sockaddr *) &sin6_server, sizeof(sin6_server));

    if (rc_bind < 0) {
        fprintf(stderr, "Erreur bind\n");
        exit(EXIT_FAILURE);
    }


    /* Mise du serveur a l’ecoute */

    int rc_listen = listen(s, 1024);

    if (rc_listen < 0) {
        fprintf(stderr, "Erreur listen\n");
        exit(EXIT_FAILURE);
    }


    /* Buffer de lecture */

    char buf_lecture[BUFFSIZE];
    memset(&buf_lecture, 0, sizeof(buf_lecture));

    while (1) {

        /* Attend une connexion */
        memset(&sin6_client, 0, sizeof(sin6_client));
        int longueur = sizeof(sin6_client);
        c = accept(s, (struct sockaddr *) &sin6_client, (socklen_t *) &longueur);
        pid_t pid = fork();

        //fils
        if (pid == 0) {
            fprintf(stdout, "Connexion d'un client\n");

            int rc_read = (int) read(c, buf_lecture, BUFFSIZE);

            if (rc_read == 0) {
                break;
            }
            if (rc_read == BUFFSIZE || rc_read < 0) {
                fprintf(stderr, "Erreur reading data\n");
                close(c);
                exit(EXIT_FAILURE);
            }

            /* Buffer d'ecriture de reponse */

            char buf_rep[BUFFSIZE];
            memset(&buf_rep, 0, sizeof(buf_rep));

            int succes = findSuccess(buf_lecture, rc_read);
            reponse(succes, buf_rep, sizeof(buf_rep));

            /* Envoie une page HTML */

            int rc_write = (int) write(c, buf_rep, sizeof(buf_rep));

            if (rc_write < 0) {
                fprintf(stderr, "Erreur writing data\n");
                close(c);
                exit(EXIT_FAILURE);
            }

            close(c);
            exit(EXIT_SUCCESS);
        }
        //pere
        close(c);
        continue;
    }

    return 0;
}

/* Message repondu en foction du succes */
void reponse(int succes, char *rep, int size) {

    int code = (succes > 1) ? 200 : 400;

    char *msg1 = (succes > 1) ? "Bien" : "echec";

    char *msg2 = (succes > 2) ? "<!DOCTYPE html>\n"
                                "<html>\n"
                                "<head>\n"
                                "</head>\n"
                                "<body>\n"
                                "<h1> Hello user </h1>\n"
                                "<p>Its work !</p>\n"
                                "</body>\n"
                                "</html>\n" : "\n";

    snprintf(rep, size,
             "HTTP/1.1 %03d %s\n"
             "Content-Type: text/html; charset=utf-8\n"
             "Connection: close\n\n%s",
             code, msg1, msg2);

}

/* determine le niveau du succes ( si succes est (0 ou 1)-> code = 400
 * si succes est (2 (HEAD) ou 3 (GET)) -> code = 200*/
int findSuccess(char *buf, int rc) {
    int succes = 0;
    char *p2 = decoupe(buf, rc);
    char *p3 = decoupe(p2, (int) (rc + buf - p2));

    if (p2 != NULL) {
        if (memcmp(buf, "GET ", strlen("GET ")) == 0) {
            succes = 1;
        }
        if (memcmp(buf, "HEAD ", strlen("HEAD ")) == 0 || succes == 1) {
            if ((p3 != NULL && memcmp(p2, "/", strlen("/")) == 0) && (memcmp(p3, "HTTP/1", strlen("HTTP/1")) == 0)) {
                succes += 2;
            }
        }
    }

    return succes;
}

/* retourne NULL si il n y a pas d'espace ou si il n y a rien apres les espaces  */
char *decoupe(char *buf, int size) {
    if (buf == NULL) {
        return NULL;
    }

    char *esp = memchr(buf, ' ', (size_t) size);
    if (esp == NULL) {
        return NULL;
    }

    esp++;

    while (esp != NULL && *esp == ' ' && esp < buf + size) {

        esp++;
    }
    if (esp == buf + size) {
        return NULL;
    }

    return esp;
}