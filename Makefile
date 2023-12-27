CC=gcc
CFLAGS=-g -I.

.PHONY: all
all: server browser

.PHONY: server
server:
	$(CC) $(CFLAGS) -o server sds/sds.c util.c server.c server-connection.c server-cli.c

.PHONY: browser
browser:
	$(CC) $(CFLAGS) -o browser sds/sds.c util.c browser.c browser-cli.c

.PHONY: clean
clean:
	rm -rf server browser
