#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "helper.h"
#include "tcp.h"

void pre_probe(char settings[][256], char *filename) {
	verb("Initializing pre-probing phase...  ");
	int sock = init_client(settings[SERVER_IP], settings[PRE_TCP], settings[PRE_TCP], SOCK_STREAM);
	verb("Initialized!\n");

	if (verbose) {
		int mtu;
		socklen_t mtu_size = sizeof(int);
		getsockopt(sock, IPPROTO_IP, IP_MTU, &mtu, &mtu_size);
		verb("MTU: %dB\n", mtu);
	}

	int i;

	get_from_file(settings, filename);
	verb("Sending settings:\n");
	for (i = 0; i < 11; i++) {
		if (send(sock, settings[i], (int)strlen(settings[i]) + 1, 0) == -1) {
			fprintf(stderr, "send error: %s\n", strerror(errno));
			verb("restarting...\n");
			while (send(sock, "restart", strlen("restart") + 1, 0) == -1) {
				continue;
			}
			i = -1;
		}
		verb("   %s\n", settings[i]);
		usleep(5000);
	}

	verb("Settings sent!\nPre-probing phase complete\n\n");
	close(sock);

	return;
}

void probe(char settings[][256]) {
	verb("Waiting for server...\n");
	sleep(2);
	verb("Initializing probing phase...  ");
	int sock = init_client(settings[SERVER_IP], settings[DST_UDP], settings[SRC_UDP], SOCK_DGRAM);
	verb("Initialized!\n");

	int yes = IP_PMTUDISC_DO;
	if (setsockopt(sock, IPPROTO_IP, IP_MTU_DISCOVER, &yes, sizeof(int)) != 0) {
		fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
	}

	int num_packets = atoi(settings[UDP_AMOUNT]);
	int size        = atoi(settings[UDP_SIZE]);

	char msg[size];
	memset(msg, 0, size);

	sleep(1);
	verb("Sending %d packets\n", num_packets);
	for (int i = 0; i < num_packets; i++) {
		msg[0] = (char) (i >> 8);
		msg[1] = (char) i;
		if (send(sock, msg, size, 0) == -1) {
			fprintf(stderr, "send error: %s\n", strerror(errno));
			exit(-1);
		}
	}

	verb("Done sending\n");

	close(sock);
}

void post_probe() {
	
}

int main(int argc, char *argv[]) {
	char settings[11][256] = {'\0'};
	int config_idx = parse_params(argc, argv);
	char *filename = malloc(256);

	verb("Starting\n");

	if (config_idx == argc) {
		get_from_file(settings, "config.ini"); // default config file
		strncpy(filename, "config.ini", 256);
		verb("Config file not specified, using default config file 'config.ini'\n");
	} else {
		get_from_file(settings, argv[config_idx]);
		strncpy(filename, argv[config_idx], 256);
		verb("Using custom config file '%s'\n", argv[config_idx]);
	}
	
	pre_probe(settings, filename);
	probe(settings);
}
