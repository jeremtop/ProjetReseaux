#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLIGNE 64

int main(int argc, char *argv[])
{
    char * hote; /* nom d'hÃ´te du  serveur */
    char * port; /* port TCP du serveur */
    char * tunnel;
    char ip[NI_MAXHOST]; /* adresse IPv4 en notation pointÃ©e */
    struct addrinfo *resol; /* struct pour la rÃ©solution de nom */
    int s; /* descripteur de socket */

    /* Traitement des arguments */
    if (argc != 4) {/* erreur de syntaxe */
        printf("Usage: %s hote port\n",argv[0]);
        exit(1);
    }
    hote = argv[1]; /* nom d'hÃ´te du  serveur */
    port = argv[2]; /* port TCP du serveur */
    tunnel = argv[3];

    /* RÃ©solution de l'hÃ´te */
    if ( getaddrinfo(hote,port,NULL, &resol) < 0 ){
        perror("rÃ©solution adresse");
        exit(2);
    }

    /* On extrait l'addresse IP */
    sprintf(ip,"%s",inet_ntoa(((struct sockaddr_in*)resol->ai_addr)->sin_addr));

    /* CrÃ©ation de la socket, de type TCP / IP */
    /* On ne considÃ¨re que la premiÃ¨re adresse renvoyÃ©e par getaddrinfo */
    if ((s = socket(resol->ai_family,resol->ai_socktype, resol->ai_protocol)) < 0) {
        perror("allocation de socket");
        exit(3);
    }
    fprintf(stderr,"le nÂ° de la socket est : %i\n",s);

    /* Connexion */
    fprintf(stderr,"Essai de connexion Ã  %s (%s) sur le port %s\n\n",
    hote,ip,port);
    if (connect(s,resol->ai_addr,sizeof(struct sockaddr_in))<0) {
        perror("connexion");
        exit(4);
    }
    freeaddrinfo(resol); /* /!\ LibÃ©ration mÃ©moire */

    /* Session */
    char tampon[MAXLIGNE + 3]; /* tampons pour les communications */
    ssize_t lu;
    int fini = 0;
    while( 1 ) {
        lu = read(tunnel,tampon,MAXLIGNE);
        tampon[lu] = '\0';
        printf("reÃ§u: %s de tun0",tampon);
        if ( fgets(tampon,MAXLIGNE - 2,stdin) == NULL ){/* entrÃ©e standard fermÃ©e */
            fini = 1;
            fprintf(stderr,"Connexion terminÃ©e !!\n");
            fprintf(stderr,"HÃ´te distant informÃ©...\n");
            shutdown(s, SHUT_WR); /* terminaison explicite de la socket
            dans le sens client -> serveur */
            /* On ne sort pas de la boucle tout de suite ... */
        }else{   /* envoi des donnÃ©es */
            send(s,tampon,strlen(tampon),0);
        }
        /* Destruction de la socket */
        close(s);

        fprintf(stderr,"Fin de la session.\n");
        return EXIT_SUCCESS;
    }
}
