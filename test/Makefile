# Build the test program
CC=gcc
INC=../src ../test
INC_PARAMS=$(INC:%=-I%) 
CFLAGS=-Os --std=c99 -Wall -Wextra $(INC_PARAMS)
LDFLAGS=
SOURCES=../src/nocli.c
OBJECTS=$(SOURCES:.c=.o)
EXAMPLE=example.out
TEST=test.out

all: $(EXAMPLE)

# Test build and run
test: $(TEST)
	./$(TEST)

$(TEST): $(OBJECTS) test.o
	$(CC) $(LDFLAGS) $^ -o $@

# Example executable    
$(EXAMPLE): $(OBJECTS) example.o
	$(CC) $(LDFLAGS) $^ -o $@

.o: %.o:%.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	-rm -f *.o $(INC:%=%/*.o) $(EXAMPLE) $(TEST)
