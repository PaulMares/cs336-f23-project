#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

int main(int argc, char *argv[]) {
	char settings[2][11][256] = {'\0'};
	read_config(settings, argv[1]);
}
