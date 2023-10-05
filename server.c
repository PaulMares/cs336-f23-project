#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "tcp.c"

void listen_loop(int sock) {
	struct sockaddr_storage client_addr;
	socklen_t addr_size = sizeof(client_addr);
	int client_fd;
	char addr[INET6_ADDRSTRLEN];

	while (1) {
		client_fd = accept(sock, (struct sockaddr *) &client_addr, &addr_size);
		if (client_fd == -1) {
			fprintf(stderr, "accept");
			continue;
		}

		inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), addr, sizeof(addr));
		printf("Got connection from %s!\n", addr);
		break;
	}

	return;
}

int main(int argc, char *argv[]) {
	int sock = init_server("7777");
	printf("socket file descriptor: %d\n", sock);
	printf("server: waiting for connection\n");
	listen_loop(sock);
}
