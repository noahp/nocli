# Compile the example for cortex-m4 with -Os and put in a library, printing size

.SILENT:

ifeq ($(origin CC),default)
CC := arm-none-eabi-gcc
endif

CCACHE = $(shell if (which ccache >/dev/null 2>&1); then echo ccache; fi)
ifeq ($(CCACHE),ccache)
CC := ccache $(CC)
endif

AR = $(shell $(CC) --print-prog-name=ar)
SIZE = $(shell $(CC) --print-prog-name=size)

# relative to repo root
BUILD_DIR = build

INC = src test
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

# relative to repo root
SOURCES = \
  src/nocli.c \
  test/example.c \

OBJECTS = $(addprefix $(BUILD_DIR)/,$(SOURCES:.c=.o))

# depfiles for tracking include changes
DEPFILES = $(OBJECTS:%.o=%.o.d)
DEPFLAGS = -MT $@ -MMD -MP -MF $@.d

LIB = $(BUILD_DIR)/libnocli_example.a

all: $(LIB)

$(LIB): $(OBJECTS)
	$(info Linking $@)
	$(AR) rcs $@ $^
	$(SIZE) --totals $@ | tee $@.size

$(BUILD_DIR)/%.o: %.c
	$(info Compiling $<)
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(DEPFLAGS) -o $@ $<

clean:
	-rm -rf build

-include $(DEPFILES)
