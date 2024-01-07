
.PHONY: all
all: build

.PHONY: build
build:
	$(MAKE) -C ./src build

.PHONY: dev
build:
	$(MAKE) -C ./src dev

.PHONY: tests
tests:
	cd ./tests/ && $(MAKE)

.PHONY: clean
clean:
	cd ./src/ && $(MAKE) clean
	cd ./tests/ && $(MAKE) clean
