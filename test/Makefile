# Build the test program
CC=gcc
INC=../src ../test
INC_PARAMS=$(INC:%=-I%) 
CFLAGS=-Os --std=c99 -Wall -Wextra $(INC_PARAMS)
CPROFFLAGS=-fprofile-arcs -ftest-coverage
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
	gcov ../src/nocli.gcno -n

$(TEST): $(OBJECTS) test.o
	$(CC) $(LDFLAGS) -fprofile-arcs $^ -o $@

# Example executable    
$(EXAMPLE): $(OBJECTS) example.o
	$(CC) $(LDFLAGS) $^ -o $@

.o: %.o:%.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	-rm -f $(INC:%=%/*.o) $(INC:%=%/*.gcda) $(INC:%=%/*.gcno) $(INC:%=%/*.gcov) $(EXAMPLE) $(TEST)
