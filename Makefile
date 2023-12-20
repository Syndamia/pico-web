.PHONY: all
all: server browser

.PHONY: server
server:
	gcc -g -o server -I. sds/sds.c util.c server.c server-connection.c server-cli.c

.PHONY: browser
browser:
	gcc -g -o browser -I. sds/sds.c util.c browser.c browser-cli.c
