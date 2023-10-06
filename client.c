#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tcp.c"

int read_config(char settings[][11][256], char *filename) {
	FILE *fp = fopen(filename, "r");
	char *key = malloc(64);
	char *value = malloc(256);
	int i = 0;

	while (fscanf(fp, "%s %*s %s", key, value) == 2) {
		strncpy(settings[0][i], key, 64);
		strncpy(settings[1][i], value, 256);
		i++;
	}
	
	fclose(fp);
	
	free(key);
	free(value);

	return i;
}

void print_settings(char settings[][11][256]) {
	for (int i = 0; i < 11; i++) {
		printf("%s: %s\n", settings[0][i], settings[1][i]);
	}
}

void message_loop(int sock) {
	char msg[1024];
	while (strcmp(msg, "quit")) {
		memset(&msg, '\0', 1024);
		printf("Message: ");
		fgets(msg, 1024, stdin);
		msg[strlen(msg) - 1] = '\0';
		send(sock, msg, strlen(msg) + 1, 0);
	}
}

int main(int argc, char *argv[]) {
	char settings[2][11][256] = {'\0'};
	read_config(settings, argv[1]);
	int sock = init_client(settings[1][0], settings[1][5]);
	message_loop(sock);
}
