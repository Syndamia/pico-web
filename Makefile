.PHONY: all
all: server browser

.PHONY: server
server:
	gcc -o server -I. sds/sds.c util.c server.c

.PHONY: browser
browser:
	gcc -o browser -I. sds/sds.c util.c browser.c
