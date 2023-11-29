PROGS = compdetect_client compdetect_server compdetect
CLIENT_OBJS = client.o helper.o connect.o
SERVER_OBJS = server.o helper.o connect.o
STANDA_OBJS = standalone.o helper.o connect.o

%.o: %.c
	gcc -g -c -o $@ $<

all: $(PROGS)

compdetect_client: $(CLIENT_OBJS)
	gcc -g -o $@ $^

compdetect_server: $(SERVER_OBJS)
	gcc -g -o $@ $^

compdetect: $(STANDA_OBJS)
	gcc -g -o $@ $^

clean:
	rm -rf $(PROGS) $(CLIENT_OBJS) $(SERVER_OBJS) $(STANDA_OBJS)
