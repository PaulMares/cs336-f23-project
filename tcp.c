#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

// get_addr()
// Calls getaddrinfo() and returns a struct addrinfo * with the addrinfo
// corresponding to that address
// params
//   char *addr - string with the address to look for, if NULL will scan itself
//	 char *port - address' port to use
// returns
//   struct addrinfo *servinfo, a linked list of addrinfo's corresponding
//   to the given addr
struct addrinfo *get_addr(char *addr, char *port) {
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if (addr == NULL) {
		hints.ai_flags = AI_PASSIVE;
	}

	if ((status = getaddrinfo(addr, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(-1);
	}

	return servinfo;
}

// init_server()
// attempts to bind a new socet to the 
// params
//   char *addr - string with the address to look for, if NULL will scan itself
//	 char *port - address' port to use
// returns
//   struct addrinfo *servinfo, a linked list of addrinfo's corresponding
//   to the given addr
int init_server(char *port) {
	int sock;
	int yes = 1;
	struct addrinfo *s;

	struct addrinfo *servinfo = get_addr(NULL, port);

	for (s = servinfo; s != NULL; s = s->ai_next) {
		if ((sock = socket(s->ai_family, s->ai_socktype, s->ai_protocol)) == -1) {
			fprintf(stderr, "socket error: %s\n", strerror(errno));
			continue;
		}
	
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
			exit(-2);
		}

		if (bind(sock, s->ai_addr, s->ai_addrlen) == -1) {
			close(sock);
			fprintf(stderr, "binding...");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo);

	if (s == NULL) {
		fprintf(stderr, "failed to bind\n");
		exit(-3);
	}

	if (listen(sock, 10) == -1) {
		fprintf(stderr, "listen error: %s\n", strerror(errno));
	}

	return sock;
}

int init_client() {
	
}
