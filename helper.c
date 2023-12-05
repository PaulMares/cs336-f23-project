#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/random.h>

#include "helper.h"

int verbose;

// read_config()
// reads configuration settings from a string
// params
//   char settings[][256] - array that will contain the settings
//	 char *sett_text      - string containing settings, each line separated by 
//							a '\n' character
// returns
//   int i, the number of settings read from sett_text
int read_config(char settings[][256], char *sett_text) {
	char *text_copy = malloc(1024);
	strncpy(text_copy, sett_text, 1024);
	char *line = strtok(text_copy, "\n");
	char *value = malloc(256);
	int i = 0;

	// reads each line from config, should be in format
	// <setting_name> = <setting_value>
	do {
		sscanf(line, "%*s %*s %255s", value);
		strncpy(settings[i], value, 256);
		i++;
	} while ((line = strtok(NULL, "\n")) != NULL);

	return i;
}

// get_from_file()
// returns contents of a file as a single string
// params
//   char *filename  - name of the file to read from
//   char *sett_text - string that will contain contents of the file
// returns
//   int len, the length of string sett_text
int get_from_file(char *filename, char *sett_text) {
	FILE *fp = fopen(filename, "r");
	
	if (fp == NULL) {
		printf("Error reading file %s: %s\n", filename, strerror(errno));
		exit(-5);
	}
	
	char *value = malloc(128);
	int len = 0;

	// append every key-value pair to sett_text
	while (fgets(value, 128, fp) != NULL) {
		strncat(sett_text, value, 256);
		len += strlen(value);
	}

	sett_text[len - 1] = '\0';
	
	fclose(fp);
	
	free(value);

	return len;
}

// verb()
// prints verbose logging statements if enabled
// params
//   char *str - format string to print
//	 ...       - values for format specifier(s) in str
void verb(char *str, ...) {
	if (verbose == 1) {
		va_list args;
        va_start(args, str);
        vprintf(str, args);
        va_end(args);
        fflush(stdout);
	}
}

// parse_params()
// parses the parameters passed in argv
// params
//   int argc     - number of parameters passed in command line
//   char *argv[] - parameters passed in command line
// returns
//   int i, the index of the first non-optional parameter
//   (should be config filename if specified, otherwise will be default)
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

// checksum()
// calculates the checksum of an array of type uint16_t
// params
//   uint16_t *header - array for which the checksum will be calculated
//   int len          - size of the array in bytes divided by 2
// returns
//   uint32_t sum, the checksum of header
uint16_t checksum(uint16_t *header, int len) {
	uint32_t sum;
	for (sum = 0; len > 0; len--) {
		sum += *header++;
	}
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return ~sum;
}

// make_high_entropy()
// packs an array with random bytes
// params
//   char msg[] - array that will hold the high-entropy message
//   int size   - size of the array in bytes
void make_high_entropy(char msg[], int size) {
	int s = 0;
	for (int i = 2; i < size; i += s) {
		s = getrandom(&msg[i], (((size - i) < 256) ? (size - i) : 256), 0);
		if (s == -1) {
			fprintf(stderr, "getrandom error: %s\n", strerror(errno));
			s = 0;
		}
	}
}
