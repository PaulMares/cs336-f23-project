#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

struct pseudo_header {
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t tcp_length;
};

void *get_in_addr(struct sockaddr *sa);
struct addrinfo *get_addr(char *addr, char *port, int socktype);
struct addrinfo *get_host(char *port, int socktype);
int init_server(char *port, int socktype);
int init_client(char *addr, char *dst_port, char *src_port, int socktype);
int init_raw();
int accept_connection(int sock, char settings[][256]);
void send_udp(int sock, int num_packets, char msg[], int size);
long recv_udp(int sock, char settings[][256], int *last_id, int *p_recv);
void pack_ip(struct iphdr *header, uint8_t ttl, uint32_t src_addr, uint32_t dst_addr);
void pack_tcp(struct tcphdr *header, uint16_t port);
struct addrinfo * make_syn(char synhdr[40], char settings[][256], int port);
