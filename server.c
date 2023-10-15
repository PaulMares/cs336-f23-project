#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>

#include "helper.h"
#include "tcp.h"

#define POLLTIME 50     // in milliseconds
#define TIMEOUT  30000  // in milliseconds

int pre_probe(char *port, char settings[][256]) {
	int timeout = TIMEOUT;
	int pres;
	char msg[256];
	int msg_size;
	struct pollfd cpfd[1];
	int i = 0;

	verb("Initializing server...  ");
	int sock = init_server(port, SOCK_STREAM);
	verb("Initialized!\n");

	verb("Socket file descriptor: %d\n", sock);
	verb("Waiting for connection\n");
	
	int client_fd = accept_connection(sock, settings);
	
	cpfd[0].fd = client_fd;
	cpfd[0].events = POLLIN;

	while (timeout > 0) {
		pres = poll(cpfd, 1, POLLTIME);

		if (pres == -1) {
			fprintf(stderr, "poll error: %s\n", strerror(errno));
			break;
		} else if (pres == 0) {
			timeout -= POLLTIME;
		} else if (cpfd[0].revents & POLLIN) {
			msg_size = recv(client_fd, msg, 256, 0);
			if (msg_size == 0) {
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
		verb("Timeout, shutting down\n");
		close(client_fd);
		close(sock);
		exit(-3);
	} else if (i != 11) {
		verb("Did not receive full settings list, shutting down\n");
		close(client_fd);
		close(sock);
		exit(-8);
	} else {
		verb("Settings received\n");
	}

	close(client_fd);
	close(sock);

	return i;
}

void probe(char settings[][256]) {
	verb("Initializing server...  ");
	int sock = init_server(settings[DST_UDP], SOCK_DGRAM);
	verb("Initialized!\n");

	struct timespec start_time;
	struct timespec last_time;

	int msg_size = atoi(settings[UDP_SIZE]);
	
	int rcv = -1;
	char msg[msg_size];
	int last_id = -1;
	
	verb("Ready to receive packets\n");
	while ((rcv = recv(sock, msg, msg_size, 0)) <= 0);
	clock_gettime(CLOCK_REALTIME, &start_time);
	clock_gettime(CLOCK_REALTIME, &last_time);
	
	while (difftime(time(NULL), last_time.tv_sec) < 2) {
		rcv = recv(sock, msg, msg_size, MSG_DONTWAIT);
		if (rcv > 0) {
			last_id = (((char) msg[0]) << 8) | ((char) msg[1]);
			clock_gettime(CLOCK_REALTIME, &last_time);
		}
	}
	verb("Done\n");
	verb("Packets received!\nLast packet ID: %d\nTime difference: %d s, %d ms\n", last_id,
			(last_time.tv_sec - start_time.tv_sec - ((last_time.tv_nsec - start_time.tv_nsec) < 0 ? 1 : 0)),
			 (((last_time.tv_nsec - start_time.tv_nsec) / 1000000) + ((last_time.tv_nsec - start_time.tv_nsec) < 0 ? 1000 : 0)));
}

void post_probe() {
	
}

int main(int argc, char *argv[]) {
	char settings[12][256];
	memset(settings, 0, 12 * 256);
	int port_idx = parse_params(argc, argv);

	verb("Starting\n");
	
	if (port_idx == argc) {
		verb("Port not specified, using default port 7777\n");
		pre_probe("7777", settings);
	} else {
		verb("Using custom port %s\n", argv[port_idx]);
		pre_probe(argv[port_idx], settings);
	}

	probe(settings);
}
