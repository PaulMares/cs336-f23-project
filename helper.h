extern int verbose;

typedef enum {
	SERVER_IP,
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
int get_from_file(char settings[][256], char *filename);
void verb(char *str, ...);
int parse_params(int argc, char *argv[]);
