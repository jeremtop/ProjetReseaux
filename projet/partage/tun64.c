#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

char tun[256];
char inip[256];
char inport[256];
char inopt[256];
char outip[256];
char outport[256];

//Serveur
void echo(int fd, char *hote, char *port, int tunnel) {
  int nread;
  char buffer[1024];
  int pid = getpid();

  while (1) {
    nread = recv(fd, buffer, 1024, 0);
    if (nread <= 0) {
      printf("nread = %d\n", nread);
      break;
    }
    printf("%d lu par serveur :\n", nread);
    for (int i = 0; i < nread; i++) {
      printf("%c", buffer[i]);
    }
    printf("\n");
	//printf("on envoie %s (%d)au tunnel %d \n",buffer,nread,tunnel);
    write(tunnel, buffer, nread);
	
  }

  close(fd);
  fprintf(stderr, "[%s:%s](%i): Termine.\n", hote, port, pid);
}

void ext_out(char *port, int tunnel) {
	printf("---SERVEUR---\n");
	int s; /* descripteur de socket */
	struct addrinfo *resol; /* résolution */

	struct addrinfo indic = {AI_PASSIVE, /* Toute interface */
                           PF_INET6,   SOCK_STREAM, 0, /* IP mode connecté */
                           0,          NULL,        NULL, NULL};
	struct sockaddr_in6 client; /* adresse de socket du client */
	client.sin6_addr = in6addr_any;

	fprintf(stderr, "Ecoute sur le port %s\n", port);
	int err = getaddrinfo(NULL, port, &indic, &resol);
	if (err < 0) {
		fprintf(stderr, "Résolution: %s\n", gai_strerror(err));
		exit (2);
	}
	// if ((sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		// perror("socket");
		// exit(1);
	// }
	/* Creation de la socket, de type TCP / IP */
	if ((s = socket(resol->ai_family, resol->ai_socktype, resol->ai_protocol))<0) {
    perror("allocation de socket");
    exit(3);
	}
	fprintf(stderr, "le n° de la socket est : %i\n", s);

	/* On rend le port reutilisable rapidement /!\ */
	int on = 1;
	if (setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))<0) {
		perror("option socket");
		exit(4);
	}
	fprintf(stderr, "Option(s) OK!\n");

	 /* Association de la socket s a l'adresse obtenue par resolution */
	if(bind(s, resol->ai_addr, sizeof(struct sockaddr_in6)) < 0) {
		perror("bind");
		exit(5);
	}
	freeaddrinfo(resol); /* /!\ Liberation mémoire */
	fprintf(stderr,"bind!\n");

	 /* la socket est prete a recevoir */
	if (listen(s, SOMAXCONN) < 0) {
		perror("listen");
		exit(6);
	}
	fprintf(stderr, "listen!\n");
	
	int len = sizeof(struct sockaddr_in6);
	
	int resolaccept;
	if ((resolaccept=accept(s, (struct sockaddr *)&client, (socklen_t *)&len)) < 0) {
		perror("accept");
		exit(7);
	}

	/* Nom reseau du client */
	char hoteclient[NI_MAXHOST];
	char portclient[NI_MAXSERV];
	err = getnameinfo((struct sockaddr *)&client, len, hoteclient, NI_MAXHOST, portclient,  NI_MAXSERV, 0);
	if (err < 0) {
		fprintf(stderr, "resolution client (%i): %s\n", resolaccept, gai_strerror(err));
	} else {
		fprintf(stderr, "accept! (%i) ip=%s port=%s\n", resolaccept, hoteclient, portclient);
	}
	/* traitement */
	echo(resolaccept, hoteclient, portclient, tunnel);
	 
}

//Client
void ext_in(char* hote, char* port, int tunnel) {
	printf("---CLIENT---\n");
	char ip[NI_MAXHOST];    /* adresse IPv4 en notation pointee */
	struct addrinfo *resol; /* struct pour la resolution de nom */
	int s;                  /* descripteur de socket */

	if (port == NULL || hote == NULL) {
		printf("Usage: client hote port\n");
		exit (1);
	}

	/* Resolution de l'hote */

	if (getaddrinfo(hote, port, NULL, &resol) < 0) {
		perror("resolution adresse");
		exit (2);
	}

	/* On extrait l'addresse IP */
	// sprintf(ip, "%s",
         // inet_ntoa(((struct sockaddr_in *)resol->ai_addr)->sin_addr));
	sprintf(ip, "%s",
          inet_ntop(resol->ai_family, resol->ai_addr, ip, NI_MAXHOST));

	printf("ip du client = %s\n", ip);

	/* Creation de la socket, de type TCP / IP */
	/* On ne considere que la premiere adresse renvoyee par getaddrinfo */
	if ((s = socket(resol->ai_family, resol->ai_socktype, resol->ai_protocol)) < 0) {
		perror("allocation de socket");
		exit (3);
	}
	fprintf(stderr, "le n° de la socket est : %i\n", s);
	/* Connexion */
	fprintf(stderr, "Essai de connexion a  hote %s (%s) sur le port %s\n\n", hote,
          ip, port);
	if (connect(s, resol->ai_addr, sizeof(struct sockaddr_in6)) < 0) {
		perror("connexion");
		exit (4);
	}
	freeaddrinfo(resol); /* /!\ Liberation memoire */

	/* Session */

	char buffer[1024];
	int nread;

	while (1) {
		
    	nread = read(tunnel, buffer, 1024);

    	if (nread <= 0) {
    	  printf("nread = %d\n", nread);
    	  break;
    	}

    	printf("%d lu par client : \n", nread);
    	for (int i = 0; i < nread; i++) {
    	  printf("%c", buffer[i]);
    	}
    	printf("\n");
		
    	send(s, buffer, nread, 0);
		
	}

	close(s);
	fprintf(stderr, "Fin de la session.\n");
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
	char system_buffer[256];
	printf("Creation de %s\n",tunname);
	tunfd = tun_alloc(tunname);
	printf("Faire la configuration de %s...\n",tunname);
	printf("./configure-tun.sh %s\n", inip);
	sprintf(system_buffer, "./configure-tun.sh %s", inip);
	system(system_buffer);
	
	if(strcmp(outip,"fc00:1234:2::36")==0) {
		printf("Ajout route vers VM3 depuis VM1-6 \n");
		//sprintf(system_buffer, "./configure-route16.sh");
		sprintf(system_buffer, "sudo ip route add 172.16.2.176/28 via 172.16.2.10 dev tun0");
		system(system_buffer);
	}
	else if(strcmp(outip,"fc00:1234:2::16")==0){
		printf("Ajout route vers VM1 depuis VM3-6 \n");
		//sprintf(system_buffer, "./configure-route36.sh");
		sprintf(system_buffer, "sudo ip route add 172.16.2.144/28 via 172.16.2.10 dev tun0");
		system(system_buffer);
	}
	else printf("Pas de route configurée\n");
	return tunfd;
}

void read_config_file(char *filname) {
  FILE *config_file = fopen(filname, "r");
  if (config_file == NULL) {
    perror("Cannot open file");
    exit(0);
  }
  fscanf(config_file, "tun=%s\n", tun);
  fscanf(config_file, "inip=%s\n", inip);
  fscanf(config_file, "inport=%s\n", inport);
  //fscanf(config_file, "options=%s\n", inopt);
  fscanf(config_file, "outip=%s\n", outip);
  fscanf(config_file, "outport=%s\n", outport);
  printf("---CONFIGURATION---\ntun=%s\ninip=%s\ninport=%s\noptions=%s\noutip=%s\noutport=%s\n",
  tun, inip, inport, inopt, outip, outport);
}

int main(int argc, char** argv) {

	/* Traitement des arguments */
	if (argc!=2) { /* erreur de syntaxe */
		printf("Usage: %s filename\n",argv[0]);
		exit(1);
	}

	read_config_file(argv[1]);

	int tunnel = create_tun(tun);

	int f = fork();
	if (f != 0){
		ext_out(inport,tunnel);
		kill(f, SIGKILL);
	}
	else {
		printf("Client : %s (port : %s)\nAppuyez sur une touche pour continuer\n", outip, outport);
		getchar();
		ext_in(outip, outport, tunnel);
		kill(getppid(), SIGKILL);
	}
	return 0;
}
