#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#define MAXLIGNE 64

//Serveur
void echo(int f, char *hote, char *port, int tunnel) {
  int nread;              
  char buffer[1024]; 
  int pid = getpid();     

  while (1) { 
    nread = recv(f, buffer, 1024, 0);

    if (nread <= 0) {
      printf("nread = %d\n", nread);
      break;
    }

    printf("%d lu par serveur :\n", nread);
    for (int i = 0; i < nread; i++) {
      printf("%c", buffer[i]);
    }
    printf("\n");

    write(tunnel, buffer, nread);
  }

  close(f);
  fprintf(stderr, "[%s:%s](%i): Termine.\n", hote, port, pid);
}

void ext_out(char *port, int tunnel) {
	printf("serveur\n");
	int sock;
	struct addrinfo *res; /* résolution */
	
	struct addrinfo hint = {AI_PASSIVE, /* Toute interface */
                           PF_INET6,   SOCK_STREAM, 0, /* IP mode connecté */
                           0,          NULL,        NULL, NULL};
	
	
	
	fprintf(stderr, "Ecoute sur le port %s\n", port);
	int err = getaddrinfo(NULL, port, &hint, &res);
	if (err < 0) {
		fprintf(stderr, "Résolution: %s\n", gai_strerror(err));
		exit (1);
	}
	
	
	if ((sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		exit(1);
	}
	
	int on = 1;
	if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))<0) {
		perror("option socket");
		exit(1);
	}
	fprintf(stderr,"Option(s) OK!\n");
	
	
	if(bind(sock, res->ai_addr, sizeof(struct sockaddr_in6)) < 0) {
		perror("bind");
		exit(1);
	}
	
	if (listen(sock, SOMAXCONN) < 0) {
		perror("listen");
		exit(1);
	}
	
	int len = sizeof(struct sockaddr_in6);
	struct sockaddr_in6 client; /* adresse de socket du client */
	client.sin6_addr = in6addr_any;
	int resaccept;
	if ((resaccept=accept(sock, (struct sockaddr *)&client, (socklen_t *)&len)) < 0) {
		perror("accept");
		exit(1);
	}
	
	char hoteclient[NI_MAXHOST];
	char portclient[NI_MAXSERV];
	err = getnameinfo((struct sockaddr *)&client, len, hoteclient, NI_MAXHOST, portclient,  NI_MAXSERV, 0);
	if (err < 0) {
		fprintf(stderr, "resolution client (%i): %s\n", resaccept, gai_strerror(err));
	} else {
		fprintf(stderr, "accept! (%i) ip=%s port=%s\n", resaccept, hoteclient, portclient);
	}
	echo(resaccept, hoteclient, portclient, tunnel);
	
}

//Client
void ext_in(char* hote, char* port, int tunnel) {
	//char * hote; /* nom d'hÃ´te du  serveur */
    //char * port; /* port TCP du serveur */
    char ip[NI_MAXHOST]; /* adresse IPv4 en notation pointÃ©e */
    struct addrinfo *resol; /* struct pour la rÃ©solution de nom */
    int s; /* descripteur de socket */

    /* RÃ©solution de l'hÃ´te */
    if ( getaddrinfo(hote,port,NULL, &resol) < 0 ){
        perror("resolution adresse");
        exit(2);
    }
	/* On extrait l'addresse IPv4 */
	sprintf(ip,"%s",inet_ntoa(((struct sockaddr_in*)resol->ai_addr)->sin_addr));
    /* On extrait l'addresse IPv6 */
    //sprintf(ip, "%s", inet_ntop(resol->ai_family, resol->ai_addr, ip, NI_MAXHOST));
	printf("ip = %s\n", ip);
    /* CrÃ©ation de la socket, de type TCP / IP */
    /* On ne considÃ¨re que la premiÃ¨re adresse renvoyÃ©e par getaddrinfo */
    if ((s = socket(resol->ai_family,resol->ai_socktype, resol->ai_protocol)) < 0) {
        perror("allocation de socket");
        exit(3);
    }
    fprintf(stderr,"le n° de la socket est : %i\n",s);

    /* Connexion */
    fprintf(stderr,"Essai de connexion a  %s (%s) sur le port %s\n\n",
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
		
		if (lu <= 0) {
			printf("read = %d\n", lu);
			break;
		}
		
		printf("%d lu par client : \n", lu);
		for (int i = 0; i < lu; i++) {
			printf("%c", tampon[i]);
		}
		printf("\n");
		send(s, tampon, lu, 0);
        // tampon[lu] = '\0';
        // printf("reÃ§u: %s de tun0",tampon);
        // if ( fgets(tampon,MAXLIGNE - 2,stdin) == NULL ){/* entrÃ©e standard fermÃ©e */
            // fini = 1;
            // fprintf(stderr,"Connexion terminÃ©e !!\n");
            // fprintf(stderr,"HÃ´te distant informÃ©...\n");
            // shutdown(s, SHUT_WR); /* terminaison explicite de la socket
            // dans le sens client -> serveur */
            // /* On ne sort pas de la boucle tout de suite ... */
        // }else{   /* envoi des donnÃ©es */
            // send(s,tampon,strlen(tampon),0);
        // }
		
        /* Destruction de la socket */
        close(s);
        fprintf(stderr,"Fin de la session.\n");
    }
}

int tun_alloc(char *dev) {
  struct ifreq ifr;
  int fd, err;

  if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
    perror("alloc tun");
    exit(-1);
  }

  memset(&ifr, 0, sizeof(ifr));

  /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
   *        IFF_TAP   - TAP device
   *
   *        IFF_NO_PI - Do not provide packet information
   */
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
  if (*dev)
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
    close(fd);
    return err;
  }
  strcpy(dev, ifr.ifr_name);

  return fd;
}

int create_tun(char* tunname) {
	int tunfd;
	printf("Creation de %s\n",tunname);
	tunfd = tun_alloc(tunname);
	printf("Faire la configuration de %s...\n",tunname);
	system("./configure-tun.sh");
	return tunfd;
}

int main(int argc, char** argv) {
	
	/* Traitement des arguments */
	if (argc!=4) { /* erreur de syntaxe */
		printf("Usage: %s nomtunnel ip port\n",argv[0]);
		exit(1);
	}
	int tunnel = create_tun(argv[1]);
	char portentree[256];
	char portsortie[256];
	char ip[256];
	sprintf(portentree,argv[3]);
	//sprintf(portsortie,argv[3]);
	sprintf(ip,argv[2]);
	int f = fork();
	if (f != 0){
		ext_out(portentree,tunnel);
	}
	else {
		ext_in(ip, portentree, tunnel);
	}
	return (0);
}