# Build the test program
CC=gcc
INC=../src
INC_PARAMS=$(INC:%=-I%) 
CFLAGS=--std=c99 -Wall -Wextra -Wpedantic $(INC_PARAMS)
LDFLAGS=
SOURCES=test.c ../src/nocli.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=test.out

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f *.o $(INC:%=%/*.o) $(EXECUTABLE)
