#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include "connect.h"
#include "helper.h"

// get_in_addr()
// Returns the internet address contained in sa
// params
//   struct sockaddr *sa - sockaddr containing the address to return
// returns
//   &(sin_addr), or NULL if the protocol is unrecognized
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		struct sockaddr_in *sai = (struct sockaddr_in *)sa;
		return &(sai->sin_addr);
	} else {
		return NULL;
	}
}

// get_addr()
// Calls getaddrinfo() and returns a struct addrinfo * with the addrinfo
//   corresponding to that address
// params
//   char *addr - string with the address to look for, if NULL will scan itself
//	 char *port - address' port to use
// returns
//   struct addrinfo *servinfo, a linked list of addrinfo's corresponding
//   to the given addr
struct addrinfo *get_addr(char *addr, char *port, int socktype) {
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = socktype;

	if (addr == NULL) {
		hints.ai_flags = AI_PASSIVE;
	}

	if ((status = getaddrinfo(addr, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(-1);
	}

	return servinfo;
}

// init_server()
// attempts to bind a new socket, then returns that socket for the server to use
// params
//   char *port   - port for the socket to bind to
//   int socktype - eithere SOCK_STREAM (TCP) or SOCK_DGRAM (UDP)
// returns
//   int sock, the file descriptor for the socket
int init_server(char *port, int socktype) {
	int sock;
	int yes = 1;
	struct addrinfo *s;
	struct addrinfo *servinfo = get_addr(NULL, port, socktype);

	for (s = servinfo; s != NULL; s = s->ai_next) {
		if ((sock = socket(s->ai_family, s->ai_socktype, s->ai_protocol)) == -1) {
			fprintf(stderr, "socket error: %s\n", strerror(errno));
			continue;
		}
	
		if (socktype == SOCK_STREAM && setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
			exit(-2);
		}

		if (bind(sock, s->ai_addr, s->ai_addrlen) == -1) {
			close(sock);
			verb("Binding...\n");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo);

	if (s == NULL) {
		fprintf(stderr, "failed to bind\n");
		exit(-3);
	}

	if (socktype == SOCK_STREAM && listen(sock, 1) == -1) {
		fprintf(stderr, "listen error: %s\n", strerror(errno));
	}

	return sock;
}

// init_client()
// attempts to connect a socket to the server
// params
//	 char *addr     - address of the server
//   char *dst_port - port on which the server is listening
//   char *src_port - port from which the client will send packets
//   int socktype   - either SOCK_STREAM (TCP) or SOCK_DGRAM (UDP)
// returns
//   int sock, the file descriptor for the socket
int init_client(char *addr, char *dst_port, char *src_port, int socktype) {
	int sock = -1;
	int bound = 1;
	int yes = 1;
	struct addrinfo *s;
	struct addrinfo *servinfo = get_addr(addr, dst_port, socktype);
	struct addrinfo *selfinfo = get_addr(NULL, src_port, socktype);

	for (s = servinfo; s != NULL; s = s->ai_next) {
		if ((sock = socket(s->ai_family, s->ai_socktype, s->ai_protocol)) == -1) {
			fprintf(stderr, "socket error: %s\n", strerror(errno));
			continue;
		}

		if (socktype == SOCK_STREAM && setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
			exit(-2);
		}

		if (bind(sock, selfinfo->ai_addr, selfinfo->ai_addrlen)) {
			fprintf(stderr, "binding error: %s\n", strerror(errno));
			bound = 0;
			break;
		}

		if (connect(sock, s->ai_addr, s->ai_addrlen) == -1) {
			close(sock);
			verb("Connecting...\n");
			continue;
		}

		break;
	}

	if (bound == 0) {
		close(sock);
		exit(-7);
	}
	
	if (sock == -1) {
		fprintf(stderr, "could not get socket: %s\n", strerror(errno));
	}

	if (s == NULL) {
		fprintf(stderr, "couldn't connect: %s\n", strerror(errno));
		exit(-3);
	}

	freeaddrinfo(servinfo);

	return sock;
}

// init_raw()
// creates a tcp raw socket
// returns
//   int sock, the file descriptor for the socket
int init_raw() {
	int sock;
	int one = 1;
	if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP)) == -1) {
		fprintf(stderr, "raw socket error: %s\n", strerror(errno));
		exit(-1);
	}
	
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(int)) < 0) {
		fprintf(stderr, "hdrincl error: %s\n", strerror(errno));
		exit(-1);
	}
	
	return sock;
}



// accept_connection()
// attempts to accept a connection from a client
// params
//   int sock             - the file descriptor for the socket that will accept
//							a connection
//   char settings[][256] - array containing the program's settings
// returns
//   int client_fd, the file descriptor for the client connection
int accept_connection(int sock, char settings[][256]) {
	struct sockaddr_storage client_addr;
	socklen_t addr_size = sizeof(client_addr);
	int client_fd = -1;

	while (client_fd == -1) {
		client_fd = accept(sock, (struct sockaddr *) &client_addr, &addr_size);
	}

	inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), settings[11], 256);
	verb("Got connection from %s!\n", settings[11]);

	return client_fd;
}

// send_udp()
// sends udp packets through an already-established connection
// params
//   int sock        - socket descriptor for the connection
//	 int num_packets - number of packets to send
//	 char msg[]      - message to be sent
//   int size        - size of msg in bytes
void send_udp(int sock, int num_packets, char msg[], int size) {
	for (uint16_t i = 0; i < num_packets; i++) {
		// places packet id in first 2 bytes
		msg[0] = (uint8_t) (i >> 8);
		msg[1] = (uint8_t) i;
		send(sock, msg, size, 0);
	}
}

// recv_udp()
// receives UDP packets from an already-established connection
// params
//   int sock             - socket descriptor for the connection
//   char settings[][256] - settings received from the client
//	 int *last_id         - will contain the last packet ID received
//	 int *p_recv          - will contain the number of packets received
// returns
//   long msec, the number of milliseconds elapsed between the first
//   and last packet received
long recv_udp(int sock, char settings[][256], int *last_id, int *p_recv) {
	struct timespec start_time;
	struct timespec last_time;

	int msg_size = atoi(settings[UDP_SIZE]);
	int rcv = -1;
	char msg[msg_size];

	// waits for first packet
	while ((rcv = recv(sock, msg, msg_size, 0)) <= 0);
	*p_recv += 1;
	clock_gettime(CLOCK_REALTIME, &start_time);
	clock_gettime(CLOCK_REALTIME, &last_time);

	// receives packets until 2 seconds elapse after last packet
	while (difftime(time(NULL), last_time.tv_sec) < 2) {
		rcv = recv(sock, msg, msg_size, MSG_DONTWAIT);
		if (rcv > 0) {
			*p_recv += 1;
			clock_gettime(CLOCK_REALTIME, &last_time);
		}
	}

	// stores ID of last packet
	*last_id = (((char) msg[0]) << 8) | ((char) msg[1]);

	// calculates ms elapsed between first and last packets
	long msec = (last_time.tv_nsec - start_time.tv_nsec) / 1000000;
	msec += ((int) difftime(last_time.tv_sec, start_time.tv_sec)) * 1000;
	return msec;
}

// pack_ip()
// packs an iphdr struct
// params
//   struct iphdr *header - iphdr to be filled
//   uint8_t ttl          - number of hops allowed for the packet
//   uint32_t src_addr    - source IP address for the packet
//   uint32_t dst_addr    - destination IP address for the packet
void pack_ip(struct iphdr *header, uint8_t ttl,
			 uint32_t src_addr, uint32_t dst_addr) {	
	header->ihl = 0x5;
	header->version = 0x4;
	header->tos = 0x0;
	header->tot_len = htons(0x28);
	header->id = htons(0x15);
	header->frag_off = htons(0x4000);
	header->ttl = ttl;
	header->protocol = 0x6;
	header->check = 0x0;
	header->saddr = src_addr;
	header->daddr = dst_addr;
}

// pack_tcp()
// packs a tcphdr struct from settings
// params
//   struct tcphdr *header - tcphdr to be filled
//   uint16_t port         - port at destination
void pack_tcp(struct tcphdr *header, uint16_t port) {
	header->source = htons(0x6F0);
	header->dest = htons(port);
	header->seq = htonl(0x6F0);
	header->ack_seq = 0x0;
	header->res1 = 0x0;
	header->doff = 0x5;
	header->fin = 0x0;
	header->syn = 0x1;
	header->rst = 0x0;
	header->psh = 0x0;
	header->ack = 0x0;
	header->urg = 0x0;
	header->res2 = 0x0;
	header->window = 0x0;
	header->check = 0x0;
	header->urg_ptr = 0x0;
}

// make_syn()
// creates and fills a TCP SYN packet
// params
//   char synhdr[40]      - char array that will contain the IP anc TCP headers
//   char settings[][256] - array containing the program's settings
//   int port             - 0 for the head packet's port, 1 for the tail's
// returns
//   struct addrinfo *dst_addrinfo, the addrinfo of the address and port
//   specified
struct addrinfo *make_syn(char synhdr[40], char settings[][256], int port) {
	char pseudo[32];
	uint32_t src_addr;
	int hort = port == 0 ? DST_TCP_H : DST_TCP_T;
	inet_pton(AF_INET, settings[SOURCE_IP], &src_addr);

	struct addrinfo *dst_addrinfo = get_addr(settings[SERVER_IP],
										 settings[hort],
										 SOCK_STREAM);

	uint32_t dst_addr = ((struct sockaddr_in *)dst_addrinfo->ai_addr)->sin_addr.s_addr;

	// fill pseudo-header for TCP checksum
	struct pseudo_header pshdr = {
		.source_address = src_addr,
		.dest_address = dst_addr,
		.placeholder = 0x0,
		.protocol = IPPROTO_TCP,
		.tcp_length = htons(20)
	};

	struct iphdr *ip = (struct iphdr *)synhdr;
	struct tcphdr *tcp = (struct tcphdr *)(synhdr + 20);

	pack_ip(ip, atoi(settings[UDP_TTL]), src_addr, dst_addr);
	pack_tcp(tcp, hort);

	// create pseudogram for TCP checksum
	memcpy(pseudo, (char *)&pshdr, sizeof(struct pseudo_header));
	memcpy(pseudo + sizeof(struct pseudo_header), tcp, 20);

	tcp->check = checksum((uint16_t *)pseudo, 16);
	ip->check = checksum((uint16_t *)synhdr, 20);

	return dst_addrinfo;
}
