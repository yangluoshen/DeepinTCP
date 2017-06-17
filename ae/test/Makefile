all: server cli

server: ../ae.c ../anet.c server.c
	$(CC) -g $^ -o $@ -std=c99 -Wall -pedantic -I../
cli: ../ae.c ../anet.c cli.c 
	$(CC) -g $^ -o $@ -std=c99 -Wall -pedantic -I../

clean:
	rm -f server cli log core
