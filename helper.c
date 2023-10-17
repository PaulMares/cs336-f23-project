#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "helper.h"

int verbose;

int read_config(char settings[][256], char *sett_text) {
	char *text_copy = malloc(1024);
	strncpy(text_copy, sett_text, 1024);
	char *line = strtok(text_copy, "\n");
	char *value = malloc(256);
	int i = 0;

	do {
		sscanf(line, "%*s %*s %255s", value);
		strncpy(settings[i], value, 256);
		i++;
	} while ((line = strtok(NULL, "\n")) != NULL);

	return i;
}

int get_from_file(char *filename, char *sett_text) {
	FILE *fp = fopen(filename, "r");
	
	if (fp == NULL) {
		printf("Error reading file %s: %s\n", filename, strerror(errno));
		exit(-5);
	}
	
	char *value = malloc(128);
	int len = 0;

	while (fgets(value, 128, fp) != NULL) {
		strncat(sett_text, value, 256);
		len += strlen(value);
	}

	sett_text[len - 1] = '\0';
	
	fclose(fp);
	
	free(value);

	return len;
}

void verb(char *str, ...) {
	if (verbose == 1) {
		va_list args;
        va_start(args, str);
        vprintf(str, args);
        va_end(args);
        fflush(stdout);
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
