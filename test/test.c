//
// test.c
//
// Run some tests on nocli.
//
#include "nocli.h"
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
    ERROR_EXIT(line);
  }
}

static void test_nocli_prompt(void) {
  // setup
  mock_output_buffer_idx = 0;
#define PROMPT_1_STRING "nocli$ "
#define PROMPT_2_STRING "prompty!"
  struct NocliPrivate nocli_private;
  struct Nocli nocli_ctx = {
      .output_stream = mock_output,
      .command_table = NULL,
      .command_table_length = 0,
      .prefix_string = PROMPT_1_STRING,
      .echo_on = true,
      .private = &nocli_private,
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
  struct NocliPrivate nocli_private;
  struct Nocli nocli_ctx = {
      .output_stream = mock_output,
      .command_table = commands,
      .command_table_length = sizeof(commands) / sizeof(commands[0]),
      .prefix_string = "nocli $",
      .echo_on = true,
      .private = &nocli_private,
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
  Nocli_Feed(&nocli_ctx, "\x1f\x80" BAD_COMMAND_STRING "11111111\x1f\x80  \n",
             sizeof("\x1f\x80" BAD_COMMAND_STRING "11111111\x1f\x80  \n") - 1);
  PRV_COMPARE_MOCK(BAD_COMMAND_RESPONSE_STRING);

// valid command, check output and command executed
#define GOOD_COMMAND_STRING "function1 arg1 \xff 3 4 5 6 7 8 9 10 11 12\n"
#define GOOD_COMMAND_RESPONSE_STRING                                           \
  NOCLI_CONFIG_ENDLINE_STRING                                                  \
  "nocli $"                                                                    \
  "function1 arg1  3 4 5 6 7 8 9 10 11 12" NOCLI_CONFIG_ENDLINE_STRING         \
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

static void test_toggling_echo(void) {
  struct NocliPrivate nocli_private;
  struct Nocli nocli_ctx = {
      .output_stream = mock_output,
      .command_table = NULL,
      .command_table_length = 0,
      .prefix_string = "nocli $",
      .echo_on = true,
      .private = &nocli_private,
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

  nocli_ctx.echo_on = false;
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
  struct NocliPrivate nocli_private;
  struct Nocli nocli_ctx = {
      .output_stream = mock_output,
      .command_table = commands,
      .command_table_length = sizeof(commands) / sizeof(commands[0]),
      .prefix_string = "nocli $",
      .echo_on = true,
      .private = &nocli_private,
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

int main(int argc, char **argv) {
  (void)argc, (void)argv;

  test_nocli_prompt();
  test_command_call();
  test_toggling_echo();
  test_help();
}
