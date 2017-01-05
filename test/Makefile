# Build the example or test binaries.
# Test run uses gcov for coverage
CC=gcc
INC=../src ../test
INC_PARAMS=$(INC:%=-I%) 
CFLAGS=-Os --std=c99 -Wall -Wextra $(INC_PARAMS)
CPROFFLAGS=-coverage
LDFLAGS=
SOURCES=../src/nocli.c
OBJECTS=$(SOURCES:.c=.o)
EXAMPLE=example.out
TEST=test.out

all: $(EXAMPLE)

# Test build and run
test: CFLAGS+=$(CPROFFLAGS)
test: $(TEST)
	./$(TEST)
	-ls ../src
	gcov --version
	gcov -o ../src ../src/nocli.c
	-rm -f test.gcda test.gcno

$(TEST): $(OBJECTS) test.o
	$(CC) $(LDFLAGS) -coverage $^ -o $@

# Example executable    
$(EXAMPLE): $(OBJECTS) example.o
	$(CC) $(LDFLAGS) $^ -o $@

.o: %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	-rm -f $(INC:%=%/*.o) $(INC:%=%/*.gcda) $(INC:%=%/*.gcno) $(INC:%=%/*.gcov) $(EXAMPLE) $(TEST)
