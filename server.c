#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#include "tcp.c"
#include "helper.c"

#define TIMEOUT 30000

int pre_probe(int sock, char settings[][256]) {
	int timeout = TIMEOUT;
	int pres;
	char msg[256];
	int msg_size;
	struct pollfd cpfd[1];
	int i = 0;

	int client_fd = accept_connection(sock);
	
	cpfd[0].fd = client_fd;
	cpfd[0].events = POLLIN;

	while (timeout > 0) {
		pres = poll(cpfd, 1, 100);

		if (pres == -1) {
			fprintf(stderr, "poll error: %s\n", strerror(errno));
			break;
		} else if (pres == 0) {
			timeout -= 100;
		} else if (cpfd[0].revents & POLLIN) {
			msg_size = recv(client_fd, msg, 256, 0);
			if (msg_size == 0 || !strcmp(msg, "shutdown")) {
				break;
			} else if (!strcmp(msg, "restart")) {
				i = 0;
			} else {
				strncpy(settings[i], msg, 256);
				i++;
				if (i >= 11) {
					break;
				}
			}
			timeout = TIMEOUT;
		}
	}

	if (timeout <= 0) {
		printf("Timeout, socket closing\n");
	} else if (msg_size == 0) {
		printf("Client disconnected\n");
	} else if (!strcmp(msg, "shutdown")) {
		printf("Client sent shutdown command\n");
	} else {
		printf("Settings received!\n");
	}

	close(client_fd);
	close(sock);

	return i;
}

void post_probe() {
	
}

int main(int argc, char *argv[]) {
	int sock;
	char settings[11][256];
	
	if (argc != 2) {
		sock = init_server("7777"); // default port
	} else {
		sock = init_server(argv[1]);
	}
	printf("socket file descriptor: %d\n", sock);
	printf("server: waiting for connection\n");
	if (pre_probe(sock, settings) != 11) {
		printf("Did not receive full settings, terminating\n");
		exit(-6);
	}
}
