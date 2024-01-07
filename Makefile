CC=gcc
CFLAGS=-g -Isrc

.PHONY: all
all: server browser

.PHONY: server
server:
	$(CC) $(CFLAGS) -o server src/sds/sds.c src/util.c src/server.c src/server-connection.c src/server-cli.c

.PHONY: browser
browser:
	$(CC) $(CFLAGS) -o browser src/sds/sds.c src/util.c src/browser.c src/browser-net.c src/browser-cli.c

.PHONY: clean
clean:
	rm -rf server browser
