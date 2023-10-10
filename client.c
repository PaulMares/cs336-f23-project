#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tcp.c"
#include "helper.c"

void message_loop(int sock) {
	char msg[1024];
	while (strcmp(msg, "quit")) {
		memset(&msg, '\0', 1024);
		printf("Message: ");
		fgets(msg, 1024, stdin);
		msg[strlen(msg) - 1] = '\0';
		send(sock, msg, strlen(msg) + 1, 0);
		if (!strcmp(msg, "shutdown")) {
			break;
		}
	}
	close(sock);
}

int pre_probe(int sock, char settings[][256]) {
	int i;

	for (i = 0; i < 11; i++) {
		if (send(sock, settings[i], strlen(settings[i]) + 1, 0) == -1) {
			fprintf(stderr, "send error: %s\n", strerror(errno));
			
		}
	}

	return i;
}

void probe() {
	
}

void post_probe() {
	
}

int main(int argc, char *argv[]) {
	char settings[11][256] = {'\0'};

	if (argc != 2) {
		read_config(settings, "config.ini"); // default config file
	} else {
		read_config(settings, argv[1]);
	}
	
	int sock = init_client(settings[0], settings[5]);
	pre_probe(sock, settings);
}
