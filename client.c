#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/random.h>

#include "helper.h"
#include "connect.h"

// pre_probe()
// pre-probing phase of the client, parses contents of config file
//   then sends unparsed file to server
// params
//   char settings[][256] - array that will contain the settings
//	 char *filename - file name of the config file
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

// probe()
// probing phase of the client, sends a train of low-entropy udp
//	 packets to the server, followed by a short pause, and then a train
//   of high-entropy udp packets
// params
//   char settings[][256] - array that contains the settings
void probe(char settings[][256]) {
	verb("Waiting for server...\n");
	sleep(1);
	verb("Initializing probing phase...  ");
	int sock = init_client(settings[SERVER_IP], settings[DST_UDP], settings[SRC_UDP], SOCK_DGRAM);
	verb("Initialized!\n");

	// don't fragment flag
	int yes = IP_PMTUDISC_DO;
	if (setsockopt(sock, IPPROTO_IP, IP_MTU_DISCOVER, &yes, sizeof(int)) != 0) {
		fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
	}

	int num_packets = atoi(settings[UDP_AMOUNT]);
	int size        = atoi(settings[UDP_SIZE]);

	// fill msg with 0
	verb("Sending %d low-entropy packets\n", num_packets);
	char msg[size];
	memset(msg, 0, size);
	send_udp(sock, num_packets, msg, size);
	verb("Done sending, waiting %s seconds before continuing...  ", settings[INTER_TIME]);

	sleep(atoi(settings[INTER_TIME]));

	// call getrandom() until msg is filled with random bytes
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

// post_probe()
// post-probing phase of the client, receives the findings of the
//   experiment from the server and displays them to the user
// params
//   char settings[][256] - array that contains the settings
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
	printf("  Compression         : %s\n", !strcmp("y", value) ? "Detected!" : "Not detected");
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

	// check if config file specified, else default to config.ini
	if (config_idx == argc) {
		strcpy(filename, "config.ini");
		verb("Config file not specified, using default config file 'config.ini'\n\n");
	} else {
		if (((int) strlen(argv[config_idx])) > 255) {
			fprintf(stderr, "config filename too long, max length is 255 characters\n");
			exit(-1);
		}
		strncpy(filename, argv[config_idx], 255);
		verb("Using custom config file '%s'\n\n", argv[config_idx]);
	}
	
	pre_probe(settings, filename);
	probe(settings);
	post_probe(settings);
}
