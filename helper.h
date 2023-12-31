#include <stdint.h>

extern int verbose;

typedef enum {
	SERVER_IP,
	SOURCE_IP,
	SRC_UDP,
	DST_UDP,
	DST_TCP_H,
	DST_TCP_T,
	PRE_TCP,
	POST_TCP,
	UDP_SIZE,
	INTER_TIME,
	UDP_AMOUNT,
	UDP_TTL
} setenum;

int read_config(char settings[][256], char *sett_text);
int get_from_file(char *filename, char *sett_text);
void verb(char *str, ...);
int parse_params(int argc, char *argv[]);
uint16_t checksum(uint16_t *header, int len);
void make_high_entropy(char msg[], int size);
