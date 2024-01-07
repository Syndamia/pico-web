RUBY=ruby
UNITY_DIR=./Unity
CMOCK_DIR=./CMock

CC=gcc
CFLAGS=-I$(UNITY_DIR)/src -I$(CMOCK_DIR)/src -I../src -I./mocks
UNILFILES=../src/sds/sds.c

.PHONY: all
all: build run

.PHONY: build
build:
	$(RUBY) $(CMOCK_DIR)/lib/cmock.rb -oCMock.yml $$(find ../src -type f -name "*.h" -not -path "../src/sds/*")
	for testfile in $$(find . -type f -name "*.tests.c")                                                 ;\
	do                                                                                                    \
	    $(RUBY) $(UNITY_DIR)/auto/generate_test_runner.rb $$testfile                                     ;\
	    includelist=$$(head -1 browser-net.tests.c | cut -c3-) ;\
	    $(CC) $(CFLAGS) -o $${testfile%.c}                                                                \
	                    $(UNILFILES) $$(find ./mocks -type f -name "*.c" | grep -Fv $$(echo "$$includelist" | sed 's/ / -e /g')) \
	                    $$(echo "$$includelist" | sed 's/ / ..\/src\//g') \
	                    helpers.c $(UNITY_DIR)/src/unity.c $(CMOCK_DIR)/src/cmock.c                                 \
	                    $$testfile $${testfile%.c}_Runner.c                                              ;\
	done

.PHONY: run
run:
	for runner in $$(find . -type f -name "*.tests") ;\
	do                                               \
	    $$runner || exit                            ;\
	done

.PHONY: clean
clean:
	$(RM) *.tests *.tests_Runner.c
