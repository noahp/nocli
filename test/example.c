//
// example.c
//
// Example usage of nocli.
//
#include "../src/nocli.h"
#include <stdio.h>
#include <string.h>

static void arg_count(int argc, char **argv) {
  (void)argv;
  printf("Arg count: %d\n", argc - 1);
}

// forward declaration, this function modifies the context
static void change_prompt(int argc, char **argv);

static void output(const char *data, size_t length) {
  printf("%.*s", (int)length, data);
}

// setup context
static const struct NocliCommand cmdlist[] = {
    {
        .name = "count-args",
        .function = arg_count,
#if NOCLI_CONFIG_HELP_COMMAND
        .help = "print number of args passed",
#endif
    },
    {
        .name = "change-prompt",
        .function = change_prompt,
#if NOCLI_CONFIG_HELP_COMMAND
        .help = "set prompt to new string",
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

static void change_prompt(int argc, char **argv) {
#define MIN(a_, b_) ((a_ < b_) ? (a_) : (b_))
  (void)argv;
  static char prefix_string[25];
  if (argc > 1) {
    const size_t len = MIN(sizeof(prefix_string) - 1, strlen(argv[1]));
    memcpy(prefix_string, argv[1], len);
    prefix_string[len] = '\0';
    nocli_ctx.prefix_string = prefix_string;
    printf("prompt updated!\n");
  }
}

int main(int argc, char **argv) {
  (void)argc, (void)argv;
  char ch;

  Nocli_Init(&nocli_ctx);

  // feed characters from getchar. capture ctrl-c.
  // turn off input buffering
  setvbuf(stdin, NULL, _IONBF, 0);
  while ((ch = (char)getchar()) != 3) {
    Nocli_Feed(&nocli_ctx, &ch, sizeof(ch));
  }

  return 0;
}
