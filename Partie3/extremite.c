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

//Serveur
void ext_out(char *port, int tunnel) {
	printf("serveur\n");
	int sock;
	struct addrinfo *res;
	
	struct addrinfo hint = {AI_PASSIVE, /* Toute interface */
                           PF_INET6,   SOCK_STREAM, 0, /* TCP IPv6 */
                           0,          NULL,        NULL, NULL};
	
	
	
	fprintf(stderr, "Ecoute sur le port %s\n", port);
	int err = getaddrinfo(NULL, port, &hint, &res);
	if (err < 0) {
		fprintf(stderr, "Résolution: %s\n", gai_strerror(err));
		exit (2);
	}
	
	
	if ((sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		exit(1);
	}
	
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
	if (getnameinfo((struct sockaddr *)&client, len, hoteclient, NI_MAXHOST, portclient,  NI_MAXSERV, 0) < 0) {
		perror("getnameinfo");
		exit(1);
	}
	
	int nread;
	char buffer[1024];
	printf("en attente\n");
	while (1) {
		
		nread = recv(tunnel, buffer, 1024, 0);
		if(nread <= 0) break;
		
		printf("%d lu par serveur =>\n", nread);
		for (int i = 0; i < nread; i++) {
			printf("%c", buffer[i]);
		}
		printf("\n");
		
		write(tunnel, buffer, nread);
		
	}
	printf("termine\n");
	close(sock);
}
//Client
// void ext-in(int tunnel) {
	// int sock;
	
	// if ((sock = socket(resol->ai_family, resol->ai_socktype, resol->ai_protocol)) < 0) {
		// perror("socket");
		// exit(1);
	// }
// }

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
	printf("Création de %s\n",tunname);
	tunfd = tun_alloc(tunname);
	printf("Faire la configuration de %s...\n",tunname);
	system("./configure-tun.sh");
	return tunfd;
}

int main(int argc, char** argv) {

	int tunnel = create_tun(argv[1]);
	
	ext_out(argv[2],tunnel);
	
	return (0);
}