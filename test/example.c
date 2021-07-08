//
// example.c
//
// Example usage of nocli.
//
#include "../src/nocli.h"
#include <stdio.h>

static void function1(int argc, char **argv) {
  (void)argv;
  printf("[Executing function1] %d\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("%s ", argv[i]);
  }
}

static void function2(int argc, char **argv) {
  (void)argv;
  printf("[Executing function2] %d\n", argc);
}

static void output(char *data, size_t length) {
  printf("%.*s", (int)length, data);
}

int main(int argc, char **argv) {
  (void)argc, (void)argv;
  char ch;

  // setup context
  static const struct NocliCommand cmdlist[] = {
      {
          .name = "function1",
          .function = function1,
          .help = "Run function 1",
      },
      {
          .name = "function2",
          .function = function2,
          .help = "Run function 2",
      },
  };
  static struct NocliPrivate nocli_private;
  static struct Nocli nocli_ctx = {
      .output_stream = output,
      .command_table = cmdlist,
      .command_table_length = sizeof(cmdlist) / sizeof(cmdlist[0]),
      .prefix_string = "nocli$ ",
      .error_string = "error, command not found",
      .echo_on = false,
      .private = &nocli_private,
  };

  Nocli_Init(&nocli_ctx);

  // feed characters from getchar. capture ctrl-c.
  // turn off input buffering
  setvbuf(stdin, NULL, _IONBF, 0);
  while ((ch = getchar()) != 3) {
    Nocli_Feed(&nocli_ctx, &ch, sizeof(ch));
  }

  return 0;
}
