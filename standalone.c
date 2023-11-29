#include <stdio.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <threads.h>

#include "helper.h"
#include "connect.h"

char settings[12][256];
struct addrinfo *head_addr;
struct addrinfo *tail_addr;

int listen_rst(void *arg) {
	int sSYN = *((int *)arg);
	verb("Thread is running\n");

	struct timespec start_time;
	struct timespec last_time;

	char msg[512];
	socklen_t addrlen = 0;
	
	do {
		recvfrom(sSYN, msg, 512, 0, head_addr->ai_addr, &addrlen);
	} while (msg[33] != 0b00010100);

	verb("GOT RST!!!!\n");

	clock_gettime(CLOCK_REALTIME, &start_time);
	clock_gettime(CLOCK_REALTIME, &last_time);

	return 1;
}

void probe(char *filename) {
	char *sett_text = malloc(1024);
	get_from_file(filename, sett_text);
	read_config(settings, sett_text);

	verb("Initializing probing phase...  ");
	int sSYN = init_raw();
	int sUDP = init_client(settings[SERVER_IP], settings[DST_UDP],
						   settings[SRC_UDP], SOCK_DGRAM);
	verb("Sockets open...  ");
	thrd_t t;
	thrd_create(&t, listen_rst, &sSYN);
	verb("Thread created...  Initialized!\nBuilding packets...  ");
	
	char syn_head[40];
	char syn_tail[40];

	int num_packets = atoi(settings[UDP_AMOUNT]);
	int size        = atoi(settings[UDP_SIZE]);

	char msg[size];
	memset(msg, 0, size);

	head_addr = make_syn(syn_head, settings, 0);
	tail_addr = make_syn(syn_tail, settings, 1);

	verb("Packets built\nSetting options...  ");
	
	int one = 1;
	uint8_t ttl = atoi(settings[UDP_TTL]);
	if (setsockopt(sSYN, IPPROTO_IP, IP_HDRINCL, &one, sizeof(int)) < 0) {
		fprintf(stderr, "hdrincl error: %s\n", strerror(errno));
		exit(-1);
	}

	if (setsockopt(sUDP, IPPROTO_IP, IP_PMTUDISC_DO, &one, sizeof(int)) < 0) {
		fprintf(stderr, "don't fragment error: %s\n", strerror(errno));
		exit(-1);
	}

	if (setsockopt(sUDP, IPPROTO_IP, IP_TTL, &ttl, sizeof(uint8_t)) < 0) {
		fprintf(stderr, "ttl set error: %s\n", strerror(errno));
		exit(-1);
	}

	verb("Options set\nSending %d low-entropy packets...  ", num_packets);

	if (sendto(sSYN, syn_head, 40, 0, head_addr->ai_addr, INET_ADDRSTRLEN) < 0) {
		fprintf(stderr, "sendto error: %s\n", strerror(errno));
		exit(-1);
	}

	send_udp(sUDP, num_packets, msg, size);

	if (sendto(sSYN, syn_tail, 40, 0, tail_addr->ai_addr, INET_ADDRSTRLEN) < 0) {
		fprintf(stderr, "sendto error: %s\n", strerror(errno));
		exit(-1);
	}

	verb("Sent!\n");

	thrd_join(t, NULL);
}

int main(int argc, char *argv[]) {
	memset(settings, 0, 12*256);
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

	probe(filename);
}
