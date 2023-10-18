#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "helper.h"
#include "connect.h"

// pre_probe()
// pre-probing phase of the server, receives the contents of the
//	 config file from the client and parses them
// params
//   char settings[][256] - array that will contain the settings
//	 char *port - port the server will listen on
int pre_probe(char settings[][256], char *port) {
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
// probe()
// probing phase of the server, receives a train of low-entropy udp
//	 packets to the server, followed by a short pause, and then a train
//   of high-entropy udp packets, and finally stores the results
// params
//   char settings[][256] - array that contains the settings
//   long res[2] - array that will contain the results
void probe(char settings[][256], long res[2]) {
	verb("Initializing probing phase...  ");
	int sock = init_server(settings[DST_UDP], SOCK_DGRAM);
	verb("Initialized!\n");

	verb("Ready to receive low-entropy packets\n");
	int last_low = -1;
	int p_recv = 0;
	long time_low = recv_udp(sock, settings, &last_low, &p_recv);
	verb("Packets received!\n# of packets received: %d\nLast packet ID: %d\nTime difference: %d s, %d ms\n",
		 p_recv, last_low, ((int) (time_low / 1000)), ((int) (time_low % 1000)));

	verb("Ready to receive high-entropy packets\n");
	int last_high = -1;
	p_recv = 0;
	long time_high = recv_udp(sock, settings, &last_high, &p_recv);
	verb("Packets received!\n# of packets received: %d\nLast packet ID: %d\nTime difference: %d s, %d ms\n",
		 p_recv, last_high, ((int) (time_high / 1000)), ((int) (time_high % 1000)));

	verb("Probing phase complete\n\n");
	close(sock);

	res[0] = time_low;
	res[1] = time_high;
}

// post_probe()
// post-probing phase of the server, calculates if compression was
//	 detected and sends its findings to the client
// params
//   char settings[][256] - array that contains the settings
//   long res[2] - array that contains the results
void post_probe(char settings[][256], long res[2]) {
	verb("Initializing post-probing phase...  ");
	int sock = init_server(settings[POST_TCP], SOCK_STREAM);
	verb("Initialized!\n");

	verb("Waiting for connection\n");
	int client_fd = accept_connection(sock, settings);

	// results structured as "c = %s|l = %li|h = %li"
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
	char *port = malloc(6);

	verb("Starting\n");

	// check if port specified, else default to 7777
	if (port_idx == argc) {
		strcpy(port, "7777");
		verb("Port not specified, using default port 7777\n");
	} else {
		if (((int) strlen(argv[port_idx])) > 5) {
			fprintf(stderr, "port too long, max length is 5 characters\n");
			exit(-1);
		}
		strncpy(port, argv[port_idx], 5);
		verb("Using custom port %s\n", argv[port_idx]);
	}

	long res[2];
	pre_probe(settings, port);
	probe(settings, res);
	post_probe(settings, res);
}
