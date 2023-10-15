#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "helper.h"

int verbose;

int read_config(char settings[][256], char *sett_text) {
	char *line = strtok(sett_text, "\n");
	char *value = malloc(256);
	int i = 0;

	do {
		sscanf(line, "%*s %*s %255s", value);
		strncpy(settings[i], value, 256);
		i++;
	} while ((line = strtok(NULL, "\n")) != NULL);

	return i;
}

int get_from_file(char settings[][256], char *filename) {
	FILE *fp = fopen(filename, "r");
	
	if (fp == NULL) {
		printf("Error reading file %s: %s\n", filename, strerror(errno));
		exit(-5);
	}
	
	char *value = malloc(128);
	char *sett_text = malloc(1024);

	while (fgets(value, 128, fp) != NULL) {
		strncat(sett_text, value, 256);
	}

	sett_text[strlen(sett_text) - 1] = '\0';
	
	fclose(fp);
	
	free(value);

	int sett_num = read_config(settings, sett_text);

	return sett_num;
}

void verb(char *str, ...) {
	if (verbose == 1) {
		va_list args;
        va_start(args, str);
        vprintf(str, args);
        va_end(args);
	}
}

int parse_params(int argc, char *argv[]) {
	int i;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			break;
		} else if (!strcmp(argv[i], "-v")) {
			verbose = 1;
		}
	}
	return i;
}
