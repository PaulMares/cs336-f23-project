PROGS = compdetect_client compdetect_server
CLIENT_OBJS = client.o
SERVER_OBJS = server.o

%.o: %.c
	gcc -g -c -o $@ $<

all: $(PROGS)

compdetect_client: $(CLIENT_OBJS)
	gcc -g -o $@ $^

compdetect_server: $(SERVER_OBJS)
	gcc -g -o $@ $^

clean:
	rm -rf $(PROGS) $(CLIENT_OBJS) $(SERVER_OBJS)
