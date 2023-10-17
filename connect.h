#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

void *get_in_addr(struct sockaddr *sa);
struct addrinfo *get_addr(char *addr, char *port, int socktype);
int init_server(char *port, int socktype);
int init_client(char *addr, char *dst_port, char *src_port, int socktype);
int accept_connection(int sock, char settings[][256]);
void send_udp(int sock, int num_packets, char msg[], int size);
long recv_udp(int sock, char settings[][256], int *last_id);
