//
// example.c
//
// Example usage of nocli.
//
#include "../src/nocli.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

static void output(const char *data, size_t length) {
  printf("%.*s", (int)length, data);
}

int main(int argc, char **argv) {
  char ch;

  // setup context
  static const struct NocliCommand cmdlist[] = {
      {
          .name = "function1",
          .function = function1,
#if NOCLI_CONFIG_HELP_COMMAND
          .help = "Run function 1",
#endif
      },
      {
          .name = "function2",
          .function = function2,
#if NOCLI_CONFIG_HELP_COMMAND
          .help = "Run function 2",
#endif
      },
  };
  static struct NocliPrivate nocli_private;
  static struct Nocli nocli_ctx = {
      .output_stream = output,
      .command_table = cmdlist,
      .command_table_length = sizeof(cmdlist) / sizeof(cmdlist[0]),
      .prefix_string = "nocli$ ",
      .echo_on = false,
      .private = &nocli_private,
  };

  Nocli_Init(&nocli_ctx);

#if defined(AFL)
  (void)argc, (void)argv;
  while (read(STDIN_FILENO, &ch, 1) > 0) {
    Nocli_Feed(&nocli_ctx, &ch, 1);
  }
#else

  // if running in non-interactive mode, feed in argv[1]
  if (argc > 1) {
    Nocli_Feed(&nocli_ctx, argv[1], strlen(argv[1]));
    Nocli_Feed(&nocli_ctx, "\n", strlen("\n"));
  } else {
    // interactive mode

    // feed characters from getchar. capture ctrl-c.
    // turn off input buffering
    setvbuf(stdin, NULL, _IONBF, 0);
    while ((ch = (char)getchar()) != 3) {
      Nocli_Feed(&nocli_ctx, &ch, sizeof(ch));
    }
  }
#endif

  return 0;
}
