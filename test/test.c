//
// test.c
//
// Run some tests on nocli.
//
#include "nocli.h"
#include <stdio.h>
#include <string.h>

#define ERROR_EXIT                                                             \
  printf(">> ERROR at line %d\n", __LINE__);                                   \
  return -1

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

static int test_nocli_prompt(void) {
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
  if (mock_output_buffer_idx !=
      sizeof(NOCLI_CONFIG_ENDLINE_STRING PROMPT_1_STRING) - 1) {
    ERROR_EXIT;
  }
  if (memcmp(mock_output_buffer, NOCLI_CONFIG_ENDLINE_STRING PROMPT_1_STRING,
             mock_output_buffer_idx) != 0) {
    printf("%.*s\n", (int)mock_output_buffer_idx, mock_output_buffer);
    ERROR_EXIT;
  }

  // modify prompt text, and feed a newline
  mock_output_buffer_idx = 0;
  nocli_ctx.prefix_string = PROMPT_2_STRING;
  Nocli_Feed(&nocli_ctx, "\n", 1);
  if (mock_output_buffer_idx !=
      sizeof(NOCLI_CONFIG_ENDLINE_STRING PROMPT_2_STRING) - 1) {
    ERROR_EXIT;
  }
  if (memcmp(mock_output_buffer, NOCLI_CONFIG_ENDLINE_STRING PROMPT_2_STRING,
             mock_output_buffer_idx) != 0) {
    printf("%.*s\n", (int)mock_output_buffer_idx, mock_output_buffer);
    ERROR_EXIT;
  }

  return 0;
}

static bool function1_iscalled = false;
static void function1(int argc, char **argv) {
  (void)argc, (void)argv;
  function1_iscalled = true;
}

static int test_command_call(void) {
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
#define BAD_COMMAND_STRING "badcommand arg1 arg2"
#define BAD_COMMAND_RESPONSE_STRING                                            \
  NOCLI_CONFIG_ENDLINE_STRING                                                  \
  "nocli $" BAD_COMMAND_STRING NOCLI_CONFIG_ENDLINE_STRING                     \
  "error, command not found" NOCLI_CONFIG_ENDLINE_STRING "nocli $"
  Nocli_Feed(&nocli_ctx, BAD_COMMAND_STRING "\n",
             sizeof(BAD_COMMAND_STRING "\n") - 1);
  if (mock_output_buffer_idx != sizeof(BAD_COMMAND_RESPONSE_STRING) - 1) {
    ERROR_EXIT;
  }
  if (memcmp(mock_output_buffer, BAD_COMMAND_RESPONSE_STRING,
             mock_output_buffer_idx) != 0) {
    ERROR_EXIT;
  }

// valid command, check output and command executed
#define GOOD_COMMAND_STRING "function1 arg1\n"
#define GOOD_COMMAND_RESPONSE_STRING                                           \
  "function1 arg1" NOCLI_CONFIG_ENDLINE_STRING "nocli $"
  mock_output_buffer_idx = 0;
  Nocli_Feed(&nocli_ctx, GOOD_COMMAND_STRING, sizeof(GOOD_COMMAND_STRING) - 1);
  if (mock_output_buffer_idx != sizeof(GOOD_COMMAND_RESPONSE_STRING) - 1) {
    printf(">> %zu %zu\n", mock_output_buffer_idx,
           sizeof(GOOD_COMMAND_RESPONSE_STRING));
    printf(">> %.*s\n", (int)mock_output_buffer_idx, mock_output_buffer);
    ERROR_EXIT;
  }
  if (memcmp(mock_output_buffer, GOOD_COMMAND_RESPONSE_STRING,
             mock_output_buffer_idx) != 0) {
    ERROR_EXIT;
  }
  if (function1_iscalled == false) {
    ERROR_EXIT;
  }

  return 0;
}

static int test_help(void) {
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
  Nocli_Feed(&nocli_ctx, "?\b\b?\n", sizeof("?\b\b?\n") - 1);

// check result- help string
#define HELP_RESULT_STRING                                                     \
  NOCLI_CONFIG_ENDLINE_STRING                                                  \
  "nocli $?\b?" NOCLI_CONFIG_ENDLINE_STRING "?" NOCLI_CONFIG_ENDLINE_STRING    \
  "help" NOCLI_CONFIG_ENDLINE_STRING "function1"                               \
  "\t"                                                                         \
  "function1 help" NOCLI_CONFIG_ENDLINE_STRING "nocli $"
  if (mock_output_buffer_idx != sizeof(HELP_RESULT_STRING) - 1) {
    printf("%d %.*s\n", (int)mock_output_buffer_idx,
           (int)mock_output_buffer_idx, mock_output_buffer);
    ERROR_EXIT;
  }
  if (memcmp(mock_output_buffer, HELP_RESULT_STRING, mock_output_buffer_idx) !=
      0) {
    printf("%.*s\n", (int)mock_output_buffer_idx, mock_output_buffer);
    ERROR_EXIT;
  }

  return 0;
}

int main(int argc, char **argv) {
  (void)argc, (void)argv;

  return (test_nocli_prompt() || test_command_call() || test_help()) ? (-1)
                                                                     : (0);
}
