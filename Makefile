# Static analysis
CC_SANA ?= clang
CFLAGS_SANA ?= --analyze -Xclang -analyzer-output=text

# Security analysis
CC_CANA ?= flawfinder
CFLAGS_CANA ?= --error-level=3

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

.PHONY: security-analysis
security-analysis:
	$(CC_CANA) $(CFLAGS_CANA) $$(find ./src -maxdepth 1 -type f -name "*.c" -o -name "*.h")

.PHONY: clean
clean:
	cd ./src/ && $(MAKE) clean
	cd ./tests/ && $(MAKE) clean
