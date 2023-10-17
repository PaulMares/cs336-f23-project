#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/random.h>

#include "helper.h"
#include "connect.h"

void pre_probe(char settings[][256], char *filename) {
	char *sett_text = malloc(1024);
	int len = get_from_file(filename, sett_text);
	read_config(settings, sett_text);
	
	verb("Initializing pre-probing phase...  ");
	int sock = init_client(settings[SERVER_IP], settings[PRE_TCP], settings[PRE_TCP], SOCK_STREAM);
	verb("Initialized!\n");

	if (verbose) {
		int mtu;
		socklen_t mtu_size = sizeof(int);
		getsockopt(sock, IPPROTO_IP, IP_MTU, &mtu, &mtu_size);
		verb("MTU: %dB\n", mtu);
	}

	verb("Sending settings...  ");

	send(sock, sett_text, len + 1, 0);

	verb("Settings sent!\nPre-probing phase complete\n\n");
	close(sock);

	return;
}

void probe(char settings[][256]) {
	verb("Waiting for server...\n");
	sleep(1);
	verb("Initializing probing phase...  ");
	int sock = init_client(settings[SERVER_IP], settings[DST_UDP], settings[SRC_UDP], SOCK_DGRAM);
	verb("Initialized!\n");

	int yes = IP_PMTUDISC_DO;
	if (setsockopt(sock, IPPROTO_IP, IP_MTU_DISCOVER, &yes, sizeof(int)) != 0) {
		fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
	}

	int num_packets = atoi(settings[UDP_AMOUNT]);
	int size        = atoi(settings[UDP_SIZE]);

	verb("Sending %d low-entropy packets\n", num_packets);
	char msg[size];
	memset(msg, 0, size);
	send_udp(sock, num_packets, msg, size);
	verb("Done sending, waiting %s seconds before continuing...  ", settings[INTER_TIME]);

	sleep(atoi(settings[INTER_TIME]));

	verb("Done waiting\nSending %d high-entropy packets\n", num_packets);
	int s = 0;
	for (int i = 2; i < size; i += s) {
		s = getrandom(&msg[i], (((size - i) < 256) ? (size - i) : 256), 0);
		if (s == -1) {
			fprintf(stderr, "getrandom error: %s\n", strerror(errno));
			s = 0;
		}
	}
	send_udp(sock, num_packets, msg, size);
	verb("Done sending\nProbing phase complete\n\n");
	
	close(sock);
}

void post_probe(char settings[][256]) {
	verb("Waiting for server...\n");
	sleep(10);
	verb("Initializing post-probing phase...  ");
	int sock = init_client(settings[SERVER_IP], settings[POST_TCP], settings[POST_TCP], SOCK_STREAM);
	verb("Initialized!\n");

	char msg[64];
	int s = recv(sock, msg, 64, 0);
	if (s <= 0) {
		close(sock);
		if (s == -1) {
			fprintf(stderr, "recv error: %s\n", strerror(errno));
		} else {
			fprintf(stderr, "socket shut down: %s\n", strerror(errno));
		}
		exit(-8);
	} else {
		verb("Results received!\nPost-probing phase complete\n\n");
	}

	close(sock);

	printf("Results:\n");
	char *result = strtok(msg, "|");
	char *value = malloc(16);
	sscanf(result, "%*s %*s %s", value);
	printf("  Compression         : %s\n", !strcmp("y", value) ? "yes" : "no");
	result = strtok(NULL, "|");
	sscanf(result, "%*s %*s %s", value);
	printf("  delta-t low entropy : %s ms\n", value);
	result = strtok(NULL, "|");
	sscanf(result, "%*s %*s %s", value);
	printf("  delta-t high entropy: %s ms\n", value);
}

int main(int argc, char *argv[]) {
	char settings[11][256] = {'\0'};
	int config_idx = parse_params(argc, argv);
	char *filename = malloc(256);

	verb("Starting\n");

	if (config_idx == argc) {
		strncpy(filename, "config.ini", 256);
		verb("Config file not specified, using default config file 'config.ini'\n\n");
	} else {
		strncpy(filename, argv[config_idx], 256);
		verb("Using custom config file '%s'\n\n", argv[config_idx]);
	}
	
	pre_probe(settings, filename);
	probe(settings);
	post_probe(settings);
}
