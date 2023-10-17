#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "helper.h"
#include "connect.h"

int pre_probe(char *port, char settings[][256]) {
	char msg[1024];
	int msg_size = 1;

	verb("Initializing pre-probing phase...  ");
	int sock = init_server(port, SOCK_STREAM);
	verb("Initialized!\n");

	verb("Waiting for connection\n");
	int client_fd = accept_connection(sock, settings);

	msg_size = recv(client_fd, msg, 1024, 0);

	int i = read_config(settings, msg);

	if (msg_size <= 0) {
		close(client_fd);
		close(sock);
		if (msg_size == -1) {
			fprintf(stderr, "recv error: %s\n", strerror(errno));
		} else {
			fprintf(stderr, "socket shut down: %s\n", strerror(errno));
		}
		exit(-8);
	} else if (i != 11) {
		fprintf(stderr, "failed to receive settings, shutting down\n");
		close(client_fd);
		close(sock);
		exit(-9);
	} else {
		verb("Settings received!\nPre-probing phase complete\n\n");
	}

	close(client_fd);
	close(sock);

	return i;
}

void probe(char settings[][256], long res[]) {
	verb("Initializing probing phase...  ");
	int sock = init_server(settings[DST_UDP], SOCK_DGRAM);
	verb("Initialized!\n");

	verb("Ready to receive low-entropy packets\n");
	int last_low = -1;
	long time_low = recv_udp(sock, settings, &last_low);
	verb("Packets received!\nLast packet ID: %d\nTime difference: %d s, %d ms\n",
		 last_low, ((int) (time_low / 1000)), ((int) (time_low % 1000)));

	verb("Ready to receive high-entropy packets\n");
	int last_high = -1;
	long time_high = recv_udp(sock, settings, &last_high);
	verb("Packets received!\nLast packet ID: %d\nTime difference: %d s, %d ms\n",
			 last_high, ((int) (time_high / 1000)), ((int) (time_high % 1000)));

	verb("Probing phase complete\n\n");
	close(sock);

	res[0] = time_low;
	res[1] = time_high;
}

void post_probe(char settings[][256], long res[2]) {
	verb("Initializing post-probing phase...  ");
	int sock = init_server(settings[POST_TCP], SOCK_STREAM);
	verb("Initialized!\n");

	verb("Waiting for connection\n");
	int client_fd = accept_connection(sock, settings);

	char msg[64];
	char buf[16];
	strcat(msg, "c = ");
	strcat(msg, ((res[1] - res[0]) >= 100) ? "y" : "n");
	strcat(msg, "|l = ");
	sprintf(buf, "%li", res[0]);
	strcat(msg, buf);
	strcat(msg, "|h = ");
	sprintf(buf, "%li", res[1]);
	strcat(msg, buf);

	verb("Sending results...  ");
	if (send(client_fd, msg, ((int) strlen(msg)) + 1, 0) == -1) {
		fprintf(stderr, "send error: %s\n", strerror(errno));
		exit(-1);
	}
	verb("Sent!\nPost-probing phase complete, shutting down\n");

	close(client_fd);
	close(sock);
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

	long res[2];
	probe(settings, res);
	post_probe(settings, res);
}
