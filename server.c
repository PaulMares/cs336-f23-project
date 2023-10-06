#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#include "tcp.c"

#define TIMEOUT 60

void listen_loop(int sock) {
	struct sockaddr_storage client_addr;
	socklen_t addr_size = sizeof(client_addr);
	int client_fd;
	char addr[INET6_ADDRSTRLEN];

	while (1) {
		client_fd = accept(sock, (struct sockaddr *) &client_addr, &addr_size);
		if (client_fd == -1) {
			continue;
		}

		inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), addr, sizeof(addr));
		printf("Got connection from %s!\n", addr);
		
		int timeout = TIMEOUT;
		int pres;
		char msg[1024];
		int msg_size;
		struct pollfd cpfd[1];
		cpfd[0].fd = client_fd;
		cpfd[0].events = POLLIN;

		while (timeout > 0) {
			pres = poll(cpfd, 1, 500);

			if (pres == -1) {
				fprintf(stderr, "poll error: %s\n", strerror(errno));
				break;
			} else if (pres == 0) {
				timeout--;
			} else if (cpfd[0].revents & POLLIN) {
				msg_size = recv(client_fd, msg, 1024, 0);
				if (msg_size == 0) {
					break;
				}
				msg[msg_size] = '\0';
				printf("Message: %s\n", msg);
				timeout = TIMEOUT;
			}
		}

		if (timeout <= 0) {
			printf("Timeout, socket closing\n");
		} else if (msg_size == 0) {
			printf("Client disconnected\n");
		}
	}

	return;
}

int main(int argc, char *argv[]) {
	int sock = init_server("7777");
	printf("socket file descriptor: %d\n", sock);
	printf("server: waiting for connection\n");
	listen_loop(sock);
}
