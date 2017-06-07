all: server client

server: server.c
	gcc -g $^ -o $@ -Wall -std=c99 -pedantic

client: client.c
	gcc -g $^ -o $@ -Wall -std=c99 -pedantic

clean:
	rm -f server client

