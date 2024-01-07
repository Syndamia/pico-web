# Compilation
CC ?= gcc
CFLAGS ?= -I.
OUT ?= ../build

PROD_CFLAGS ?= -O2
DEV_CFLAGS ?= -g

.PHONY: build
build: CFLAGS += $(PROD_CFLAGS)
build: server browser

.PHONY: dev
dev: CFLAGS += $(DEV_CFLAGS)
dev: server browser

.PHONY: prod-server
prod-server: CFLAGS += $(PROD_CFLAGS)
prod-server: server

.PHONY: server
server:
	mkdir -p $(OUT)
	$(CC) $(CFLAGS) -o $(OUT)/server sds/sds.c util.c server.c server-connection.c server-cli.c

.PHONY: browser
browser:
	mkdir -p $(OUT)
	$(CC) $(CFLAGS) -o $(OUT)/browser sds/sds.c util.c browser.c browser-net.c browser-cli.c

.PHONY: clean
clean:
	$(RM) $(OUT)
