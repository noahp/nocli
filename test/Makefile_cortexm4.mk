# Compile the example for cortex-m4 with -Os and put in a library, printing size

ifeq ($(origin CC),default)
CC := arm-none-eabi-gcc
endif

AR = $(shell $(CC) --print-prog-name=ar)
SIZE = $(shell $(CC) --print-prog-name=size)

INC = ../src ../test
CFLAGS += \
  -Os \
  -mcpu=cortex-m4 \
  -mfloat-abi=hard \
  -mfpu=fpv4-sp-d16 \
  -mlittle-endian \
  -mthumb \
  -ggdb3 \
  --std=c99 \
  -Wall \
  -Wextra \
  $(INC:%=-I%)

SOURCES = ../src/nocli.c
OBJECTS = $(SOURCES:.c=.o)
LIB = libnocli_example.a

all: $(LIB)

$(LIB): $(OBJECTS) example.o
	$(AR) rcs $@ $^
	$(SIZE) $@ | tee $@.size

.o: %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	-rm -f $(INC:%=%/*.o) $(INC:%=%/*.gcda) $(INC:%=%/*.gcno) $(INC:%=%/*.gcov) $(EXAMPLE) $(TEST)
