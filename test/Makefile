# Build the example or test binaries.
# Test run uses gcov for coverage
CC ?= gcc
GCOV ?= gcov
INC = ../src ../test
INC_PARAMS = $(INC:%=-I%)
CFLAGS += -Os -ggdb3 --std=c99 -Wall -Wextra $(INC_PARAMS) -fsanitize=address -fsanitize=undefined
CPROFFLAGS = -coverage
LDFLAGS +=
SOURCES = ../src/nocli.c
OBJECTS = $(SOURCES:.c=.o)
EXAMPLE = example.out
TEST = test.out

all: $(EXAMPLE)

# Test build and run
test: CFLAGS+=$(CPROFFLAGS)
test: $(TEST)
# disable intercepting strnlen, since we use our own implementation
	ASAN_OPTIONS=intercept_strnlen=0 ./$(TEST)
	-ls ../src
	$(GCOV) --version
	$(GCOV) -o ../src ../src/nocli.c
	-rm -f test.gcda test.gcno

$(TEST): $(OBJECTS) test.o
	$(CC) $(CFLAGS) $(LDFLAGS) -coverage $^ -o $@

# Example executable
$(EXAMPLE): $(OBJECTS) example.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

.o: %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	-rm -f $(INC:%=%/*.o) $(INC:%=%/*.gcda) $(INC:%=%/*.gcno) $(INC:%=%/*.gcov) $(EXAMPLE) $(TEST)
