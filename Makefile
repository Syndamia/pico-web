.PHONY: all
all: server browser

.PHONY: server
server:
	gcc -o server -I. util.c server.c

.PHONY: browser
browser:
	gcc -o browser -I. util.c browser.c
