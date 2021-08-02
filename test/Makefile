# Build the example or test binaries.
# Test run uses gcov for coverage

.SILENT:

# relative to repo root
BUILD_DIR = build

CC ?= gcc
GCOV ?= gcov
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

ifeq ($(DISABLE_ASAN),)
CFLAGS += \
  -fsanitize=address \
  -fsanitize=undefined \
  -fno-sanitize-recover=all

endif

# only enable gcc static analyzer on gcc
ifneq ($(filter gcc,$(shell $(CC) --version )),)
CFLAGS += \
  -fanalyzer \

endif

CPROFFLAGS = -coverage
LDFLAGS +=

# relative to repo root
SOURCES = \
  src/nocli.c \

# to test multiple configurations, we build multiple copies of the test
# executable with different compiler flags
TEST_TARGETS := test1 test2

# test2 disables quoted args support, testing the simpler tokenizer
$(BUILD_DIR)/test2/test-executable: CFLAGS+=-DNOCLI_QUOTED_ARGS_SUPPORT=0

# all of the build targets, used to generate compilation rules below
BUILD_TARGETS := $(TEST_TARGETS) example fuzzing

OBJECTS = $(SOURCES:%.c=%.o)

# depfiles for tracking include changes
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
DEPFILES = $(call rwildcard,$(BUILD_DIR),*.d)
DEPFLAGS = -MT $@ -MMD -MP -MF $@.d

EXAMPLE = $(BUILD_DIR)/example/example
TEST1 = $(BUILD_DIR)/test.out

all: $(EXAMPLE)

NO_LCOV ?= 0

# Run all the test builds, merge lcov at the end
.PHONY: test
test: CFLAGS+=$(CPROFFLAGS)
test: $(TEST_TARGETS:%=$(BUILD_DIR)/%/coverage.info)
ifeq ($(NO_LCOV),0)
	lcov --rc lcov_branch_coverage=1 $(addprefix -a ,$^) -o build/lcov.info -q
	genhtml --branch-coverage --highlight --legend -o test_coverage \
		-t "coverage" --num-spaces 4 build/lcov.info -o \
		$(BUILD_DIR)/test_coverage/
	@echo See coverage output in file://$(abspath $(BUILD_DIR)/test_coverage/index.html)
endif

# the test build artifacts are in separate directories
$(BUILD_DIR)/%/coverage.info: $(BUILD_DIR)/%/test-executable
	$(info Running test...)
	./$^
	$(info Generating coverage file $@)
	lcov --gcov-tool $(GCOV) --rc lcov_branch_coverage=1 \
		--directory $(dir $<)/src -c -o $@ -q

OBJS = $(addprefix $(BUILD_DIR)/%/,$(OBJECTS))

$(BUILD_DIR)/%/test-executable: $(OBJS) $(BUILD_DIR)/%/test/test.o
	$(info Linking $@)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -coverage $^ -o $@

$(BUILD_DIR)/%/example: $(OBJS) $(BUILD_DIR)/%/test/example.o
	$(info Linking $@)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

# generate pattern rules for each target build dir
define define_compile_rules
$(BUILD_DIR)/$(1)/%.o: %.c
	$$(info Compiling $$<)
	mkdir -p $$(dir $$@)
	$$(CC) -c $$(CFLAGS) $$(DEPFLAGS) -o $$@ $$<
endef

$(foreach target,$(BUILD_TARGETS),$(eval $(call define_compile_rules,$(target))))

# fuzz testing target; use the clang compiler and append a few additional CFLAGS
.PHONY: fuzz
fuzz: CC = clang
fuzz: CFLAGS += -DFUZZING -fsanitize-address-use-after-scope -fsanitize=address,undefined,fuzzer
fuzz: $(BUILD_DIR)/fuzzing/example
	./$(BUILD_DIR)/fuzzing/example

.PHONY: clean
clean:
	-rm -rf build

-include $(DEPFILES)
