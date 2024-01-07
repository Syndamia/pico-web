# Static analysis
CC_SANA ?= clang
CFLAGS_SANA ?= --analyze -Xclang -analyzer-output=text

.PHONY: all
all: build

.PHONY: build
build:
	$(MAKE) -C ./src build

.PHONY: dev
dev:
	$(MAKE) -C ./src dev

.PHONY: tests
tests:
	$(MAKE) -C ./tests

.PHONY: static-analysis
static-analysis:
	$(CC_SANA) $(CFLAGS_SANA) ./src/*

.PHONY: clean
clean:
	cd ./src/ && $(MAKE) clean
	cd ./tests/ && $(MAKE) clean
