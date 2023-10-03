CLIENT_OBJS = client.o

%.o: %.c
	gcc -g -c -o $@ $<

client: $(CLIENT_OBJS)
	gcc -g -o $@ $^

clean:
	rm -rf client $(CLIENT_OBJS)
