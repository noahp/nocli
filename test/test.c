//
// test.c
//
// Run some tests on nocli.
//
#define _GNU_SOURCE // for asprintf

#include "../nocli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_EXIT(line_)                                                      \
  printf(">> ERROR at line %d\n", line_);                                      \
  exit(-1)

#if !defined(MIN)
#define MIN(a, b) ((a) < (b)) ? (a) : (b)
#endif
static size_t mock_output_buffer_idx = 0;
static char mock_output_buffer[1024];
static void mock_output(const char *data, size_t length) {
  length = MIN(length, sizeof(mock_output_buffer) - mock_output_buffer_idx);
  memcpy(&mock_output_buffer[mock_output_buffer_idx], data, length);
  mock_output_buffer_idx += length;
}

static void print_hex(const char *data, size_t length) {
  for (size_t i = 0; i < length; i++) {
    printf("%02x", data[i]);
  }
}

#define PRV_COMPARE_MOCK(a_)                                                   \
  prv_compare_strings(a_, mock_output_buffer, mock_output_buffer_idx, __LINE__)
static void prv_compare_strings(const char *expected, const char *actual,
                                size_t actual_length, int line) {
  bool fail = false;
  if (actual_length != strlen(expected)) {
    printf(">>> bad length, expected: %zu / actual: %zu\n", strlen(expected),
           actual_length);
    fail = true;
  } else if (memcmp(expected, actual, actual_length) != 0) {
    fail = true;
  }

  if (fail) {
    printf(">>> expected:\n%s\n<<< actual:\n%.*s\n", expected,
           (int)actual_length, actual);
    print_hex(expected, strlen(expected));
    printf("\n");
    print_hex(actual, actual_length);
    printf("\n");
    ERROR_EXIT(line);
  }
}

static void test_nocli_prompt(void) {
  // setup
  mock_output_buffer_idx = 0;
#define PROMPT_1_STRING "nocli$ "
#define PROMPT_2_STRING "prompty!"
  struct Nocli nocli_ctx = {
      .output_stream = mock_output,
      .command_table = NULL,
      .command_table_length = 0,
      .prefix_string = PROMPT_1_STRING,
#if NOCLI_RUNTIME_ECHO_CONTROL
      .echo_on = true,
#endif
  };

  Nocli_Init(&nocli_ctx);

  // check result
  PRV_COMPARE_MOCK(NOCLI_CONFIG_ENDLINE_STRING PROMPT_1_STRING);

  // modify prompt text, and feed a newline
  mock_output_buffer_idx = 0;
  nocli_ctx.prefix_string = PROMPT_2_STRING;
  Nocli_Feed(&nocli_ctx, "\n", 1);
  PRV_COMPARE_MOCK(NOCLI_CONFIG_ENDLINE_STRING PROMPT_2_STRING);
}

static bool function1_iscalled = false;
static void function1(int argc, char **argv) {
  (void)argc, (void)argv;
  function1_iscalled = true;
}

static void test_command_call(void) {
  // setup
  mock_output_buffer_idx = 0;
  function1_iscalled = false;
  struct NocliCommand commands[] = {{
      .name = "function1",
      .function = function1,
      .help = "function1 help",
  }};
  struct Nocli nocli_ctx = {
      .output_stream = mock_output,
      .command_table = commands,
      .command_table_length = sizeof(commands) / sizeof(commands[0]),
      .prefix_string = "nocli $",
#if NOCLI_RUNTIME_ECHO_CONTROL
      .echo_on = true,
#endif
  };

  Nocli_Init(&nocli_ctx);

  // test zero passed data
  Nocli_Feed(&nocli_ctx, NULL, 0);

// feed invalid command, check output
#define BAD_COMMAND_STRING                                                     \
  "badcommand arg1 arg2 "                                                      \
  "11111111111111111111111111111111111111111111111111111111111111111111111111" \
  "11111111111111111111111111111111"
#define BAD_COMMAND_RESPONSE_STRING                                            \
  NOCLI_CONFIG_ENDLINE_STRING                                                  \
  "nocli $" BAD_COMMAND_STRING NOCLI_CONFIG_ENDLINE_STRING                     \
  "error, command not found" NOCLI_CONFIG_ENDLINE_STRING "nocli $"
  // feed an overlong command string, confirm it truncates
  Nocli_Feed(&nocli_ctx, BAD_COMMAND_STRING "11111111 \n",
             sizeof(BAD_COMMAND_STRING "11111111 \n") - 1);
  PRV_COMPARE_MOCK(BAD_COMMAND_RESPONSE_STRING);

// valid command, check output and command executed
#define GOOD_COMMAND_STRING "function1 arg1 3 4 5 6 7 8 9 10 11 12\n"
#define GOOD_COMMAND_RESPONSE_STRING                                           \
  NOCLI_CONFIG_ENDLINE_STRING                                                  \
  "nocli $"                                                                    \
  "function1 arg1 3 4 5 6 7 8 9 10 11 12" NOCLI_CONFIG_ENDLINE_STRING          \
  "nocli $"
  mock_output_buffer_idx = 0;
  Nocli_Init(&nocli_ctx);
  Nocli_Feed(&nocli_ctx, GOOD_COMMAND_STRING, sizeof(GOOD_COMMAND_STRING) - 1);
  PRV_COMPARE_MOCK(GOOD_COMMAND_RESPONSE_STRING);
  if (function1_iscalled == false) {
    ERROR_EXIT(__LINE__);
  }

  // test with no command table
  nocli_ctx.command_table_length = 0,
#define NO_COMMANDS_STRING                                                     \
  NOCLI_CONFIG_ENDLINE_STRING                                                  \
  "nocli $" BAD_COMMAND_STRING NOCLI_CONFIG_ENDLINE_STRING "nocli $"

  mock_output_buffer_idx = 0;
  Nocli_Init(&nocli_ctx);
  Nocli_Feed(&nocli_ctx, BAD_COMMAND_STRING "\n",
             sizeof(BAD_COMMAND_STRING "\n") - 1);
  PRV_COMPARE_MOCK(NO_COMMANDS_STRING);
}

static void print_args(int argc, char **argv) {
  if (argc > 1) {
    argc--;
    argv++;
    mock_output("\n", 1);
    while (argc--) {
      char *ptr;
      int len = asprintf(&ptr, "%s\n", *argv++);

      mock_output(ptr, len);

      free(ptr);
    }
  }
}

static void test_arg_splitting(void) {
  // setup
  mock_output_buffer_idx = 0;
  struct NocliCommand commands[] = {{
      .name = "print_args",
      .function = print_args,
      .help = "print_args help",
  }};
  struct Nocli nocli_ctx = {
      .output_stream = mock_output,
      .command_table = commands,
      .command_table_length = sizeof(commands) / sizeof(commands[0]),
      .prefix_string = "nocli $",
#if NOCLI_RUNTIME_ECHO_CONTROL
      .echo_on = true,
#endif
  };

  // some splits
  struct testvec {
    char *name;
    char command_string[1024];
    char expected_response_string[1024];
  } testvec[] = {
#if NOCLI_QUOTED_ARGS_SUPPORT
      {
          .name = "test_quoted_splits",
          .command_string = "print_args \" 1' '\" \"'2  \n",
          .expected_response_string = NOCLI_CONFIG_ENDLINE_STRING
          "nocli $"
          "print_args \" 1' '\" \"'2  "
          "\n"
          " 1' '\n"
          "'2  "
          "\n" NOCLI_CONFIG_ENDLINE_STRING "nocli $",

      },
      {
          .name = "test_quoted_splits",
          .command_string = "print_args 0 1 2 3 4 5 6 7 8 \" \n",
          .expected_response_string = NOCLI_CONFIG_ENDLINE_STRING
          "nocli $"
          "print_args 0 1 2 3 4 5 6 7 8 \" "
          "\n"
          "0\n"
          "1\n"
          "2\n"
          "3\n"
          "4\n"
          "5\n"
          "6\n"
          "7\n"
          "8\n" NOCLI_CONFIG_ENDLINE_STRING "nocli $",
      },
      {
          .name = "test_quoted_splits",
          .command_string = "print_args 0 1 2 3 4 5 6 7 8 \"'\n",
          .expected_response_string = NOCLI_CONFIG_ENDLINE_STRING
          "nocli $"
          "print_args 0 1 2 3 4 5 6 7 8 \"'"
          "\n"
          "0\n"
          "1\n"
          "2\n"
          "3\n"
          "4\n"
          "5\n"
          "6\n"
          "7\n"
          "8\n" NOCLI_CONFIG_ENDLINE_STRING "nocli $",
      },
#endif
      {
          .name = "test_normal_splits",
          .command_string = "print_args one  two   three   \n",
          .expected_response_string = NOCLI_CONFIG_ENDLINE_STRING
          "nocli $"
          "print_args one  two   three   \n"
          "one\n"
          "two\n"
          "three\n" NOCLI_CONFIG_ENDLINE_STRING "nocli $",
      },
  };

  for (size_t i = 0; i < sizeof(testvec) / sizeof(*testvec); i++) {
    mock_output_buffer_idx = 0;
    Nocli_Init(&nocli_ctx);
    Nocli_Feed(&nocli_ctx, testvec[i].command_string,
               strlen(testvec[i].command_string));
    PRV_COMPARE_MOCK(testvec[i].expected_response_string);
  }
}

static void test_toggling_echo(void) {
  struct Nocli nocli_ctx = {
      .output_stream = mock_output,
      .command_table = NULL,
      .command_table_length = 0,
      .prefix_string = "nocli $",
#if NOCLI_RUNTIME_ECHO_CONTROL
      .echo_on = true,
#endif
  };
  mock_output_buffer_idx = 0;
  Nocli_Init(&nocli_ctx);
  // test entering '?' command, with multiple backspaces; we should only get
  // backspaces until the active buffer is blank
  Nocli_Feed(&nocli_ctx, "?\b\b\x7f?\n", strlen("?\b\b\x7f?\n"));

  // check result- help string
  const char *expecteds[] = {
      NOCLI_CONFIG_ENDLINE_STRING "nocli $?\b?" NOCLI_CONFIG_ENDLINE_STRING
                                  "?" NOCLI_CONFIG_ENDLINE_STRING
                                  "help" NOCLI_CONFIG_ENDLINE_STRING "nocli $",
      NOCLI_CONFIG_ENDLINE_STRING "nocli $" NOCLI_CONFIG_ENDLINE_STRING
                                  "?" NOCLI_CONFIG_ENDLINE_STRING
                                  "help" NOCLI_CONFIG_ENDLINE_STRING "nocli $",
  };

  PRV_COMPARE_MOCK(expecteds[0]);

#if NOCLI_RUNTIME_ECHO_CONTROL
  nocli_ctx.echo_on = false;
#endif

  mock_output_buffer_idx = 0;
  Nocli_Init(&nocli_ctx);
  // test entering '?' command, with multiple backspaces; we should only get
  // backspaces until the active buffer is blank
  Nocli_Feed(&nocli_ctx, "?\b\b?\r", strlen("?\b\b?\r"));

  // check result- help string
  PRV_COMPARE_MOCK(expecteds[1]);
}

static void test_help(void) {
  // setup
  mock_output_buffer_idx = 0;
  struct NocliCommand commands[] = {{
      .name = "function1",
      .function = function1,
      .help = "function1 help",
  }};
  struct Nocli nocli_ctx = {
      .output_stream = mock_output,
      .command_table = commands,
      .command_table_length = sizeof(commands) / sizeof(commands[0]),
      .prefix_string = "nocli $",
#if NOCLI_RUNTIME_ECHO_CONTROL
      .echo_on = true,
#endif
  };

  Nocli_Init(&nocli_ctx);
  // test entering '?' command, with multiple backspaces; we should only get
  // backspaces until the active buffer is blank
  Nocli_Feed(&nocli_ctx, "?\b\b?\n", strlen("?\b\b?\n"));

// check result- help string
#define HELP_RESULT_STRING                                                     \
  NOCLI_CONFIG_ENDLINE_STRING                                                  \
  "nocli $?\b?" NOCLI_CONFIG_ENDLINE_STRING "?" NOCLI_CONFIG_ENDLINE_STRING    \
  "help" NOCLI_CONFIG_ENDLINE_STRING "function1"                               \
  "\t"                                                                         \
  "function1 help" NOCLI_CONFIG_ENDLINE_STRING "nocli $"
  PRV_COMPARE_MOCK(HELP_RESULT_STRING);

  // test 'help' command too
  mock_output_buffer_idx = 0;
#define HELP_RESULT_STRING_2                                                   \
  NOCLI_CONFIG_ENDLINE_STRING                                                  \
  "nocli $help" NOCLI_CONFIG_ENDLINE_STRING "?" NOCLI_CONFIG_ENDLINE_STRING    \
  "help" NOCLI_CONFIG_ENDLINE_STRING "function1"                               \
  "\t"                                                                         \
  "function1 help" NOCLI_CONFIG_ENDLINE_STRING "nocli $"
  Nocli_Init(&nocli_ctx);
  Nocli_Feed(&nocli_ctx, "help\n", strlen("help\n"));
  PRV_COMPARE_MOCK(HELP_RESULT_STRING_2);
}

static void test_non_printable_characters(void) {
  // setup - test that non-printable characters are filtered out
  mock_output_buffer_idx = 0;
  function1_iscalled = false;
  struct NocliCommand commands[] = {{
      .name = "test",
      .function = function1,
      .help = "test help",
  }};
  struct Nocli nocli_ctx = {
      .output_stream = mock_output,
      .command_table = commands,
      .command_table_length = sizeof(commands) / sizeof(commands[0]),
      .prefix_string = "$ ",
#if NOCLI_RUNTIME_ECHO_CONTROL
      .echo_on = true,
#endif
  };

  Nocli_Init(&nocli_ctx);

  // Feed command with non-printable characters (ASCII < 32; 127 is DEL)
  // These should be filtered out and not appear in output or buffer
  const char *input_with_nonprintable = "te\x01\x02\x1fst\n";
  Nocli_Feed(&nocli_ctx, input_with_nonprintable,
             strlen(input_with_nonprintable));

  // Expected: non-printable chars are dropped, only "test" remains
#define NON_PRINTABLE_RESULT                                                   \
  NOCLI_CONFIG_ENDLINE_STRING "$ test" NOCLI_CONFIG_ENDLINE_STRING "$ "
  PRV_COMPARE_MOCK(NON_PRINTABLE_RESULT);

  // Verify command was executed (proves "test" was correctly parsed)
  if (function1_iscalled == false) {
    ERROR_EXIT(__LINE__);
  }
}

static void test_buffer_overflow_protection(void) {
  // Test that when buffer is full, additional characters are dropped
  mock_output_buffer_idx = 0;
  struct Nocli nocli_ctx = {
      .output_stream = mock_output,
      .command_table = NULL,
      .command_table_length = 0,
      .prefix_string = "",
#if NOCLI_RUNTIME_ECHO_CONTROL
      .echo_on = true,
#endif
  };

  Nocli_Init(&nocli_ctx);

  // Create a string that's exactly NOCLI_CONFIG_MAX_COMMAND_LENGTH - 1 chars
  // (leaving room for null terminator)
  char long_command[NOCLI_CONFIG_MAX_COMMAND_LENGTH + 20];
  memset(long_command, 'A', sizeof(long_command) - 1);
  long_command[sizeof(long_command) - 1] = '\0';

  // Feed the overlong string
  Nocli_Feed(&nocli_ctx, long_command, strlen(long_command));

  // Feed a newline to process
  Nocli_Feed(&nocli_ctx, "\n", 1);

  // Verify output contains only MAX_COMMAND_LENGTH-1 'A's (buffer truncation)
  // Output should be: ENDLINE + (MAX_LENGTH-1 A's) + ENDLINE
  char expected[NOCLI_CONFIG_MAX_COMMAND_LENGTH + 20];
  int offset = 0;
  memcpy(expected + offset, NOCLI_CONFIG_ENDLINE_STRING,
         strlen(NOCLI_CONFIG_ENDLINE_STRING));
  offset += strlen(NOCLI_CONFIG_ENDLINE_STRING);
  memset(expected + offset, 'A', NOCLI_CONFIG_MAX_COMMAND_LENGTH - 1);
  offset += NOCLI_CONFIG_MAX_COMMAND_LENGTH - 1;
  memcpy(expected + offset, NOCLI_CONFIG_ENDLINE_STRING,
         strlen(NOCLI_CONFIG_ENDLINE_STRING));
  offset += strlen(NOCLI_CONFIG_ENDLINE_STRING);

  PRV_COMPARE_MOCK(expected);
}

int main(int argc, char **argv) {
  (void)argc, (void)argv;

  test_nocli_prompt();
  test_command_call();
  test_toggling_echo();
  test_arg_splitting();
  test_help();
  test_non_printable_characters();
  test_buffer_overflow_protection();
}
