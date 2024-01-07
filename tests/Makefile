RUBY=ruby
UNITY_DIR=./Unity
CC=gcc
CFLAGS=-I$(UNITY_DIR)/src

.PHONY: all
all: build run

.PHONY: build
build:
	for testfile in $$(find . -type f -name "*.tests.c")                                                ;\
	do                                                                                                   \
	    $(RUBY) $(UNITY_DIR)/auto/generate_test_runner.rb $$testfile                                    ;\
	    $(CC) $(CFLAGS) -o $${testfile%.c} $(UNITY_DIR)/src/unity.c $$testfile $${testfile%.c}_Runner.c ;\
	done

.PHONY: run
run:
	for runner in $$(find . -type f -name "*.tests") ;\
	do                                               \
	    $$runner || exit                            ;\
	done
