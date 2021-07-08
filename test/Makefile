# Build the example or test binaries.
# Test run uses gcov for coverage

.SILENT:

# relative to repo root
BUILD_DIR = build

CC ?= gcc
INC = src test
INC_PARAMS = $(INC:%=-I%)
CFLAGS += \
  -O0 \
  -ggdb3 \
  --std=c99 \
  -Wall \
  -Wextra \
  -Werror \
  $(INC_PARAMS) \
  -fsanitize=address \
  -fsanitize=undefined \

CPROFFLAGS = -coverage
LDFLAGS +=

# relative to repo root
SOURCES = \
  src/nocli.c \

OBJECTS = $(addprefix $(BUILD_DIR)/,$(SOURCES:.c=.o))

# depfiles for tracking include changes
DEPFILES = $(OBJECTS:%.o=%.o.d)
DEPFLAGS = -MT $@ -MMD -MP -MF $@.d

EXAMPLE = $(BUILD_DIR)/example.out
TEST = $(BUILD_DIR)/test.out

all: $(EXAMPLE)

NO_LCOV ?= 0

# Test build and run
.PHONY: test
test: CFLAGS+=$(CPROFFLAGS)
test: $(TEST)
# disable intercepting strnlen, since we use our own implementation
	$(info Running test...)
	ASAN_OPTIONS=intercept_strnlen=0 ./$(TEST)
ifeq ($(NO_LCOV),0)
	lcov --rc lcov_branch_coverage=1 --directory ./build/src -c -o build/lcov.info
	genhtml --branch-coverage --highlight --legend -o test_coverage \
		-t "coverage" --num-spaces 4 build/lcov.info -o \
		$(BUILD_DIR)/test_coverage/
	@echo See coverage output in file://$(abspath $(BUILD_DIR)/test_coverage/index.html)
endif

$(TEST): $(OBJECTS) build/test/test.o
	$(info Linking $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -coverage $^ -o $@

# Example executable
$(EXAMPLE): $(OBJECTS) build/test/example.o
	$(info Linking $@)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c
	$(info Compiling $<)
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(DEPFLAGS) -o $@ $<

clean:
	-rm -rf build
