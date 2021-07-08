//
// nocli.c
//
// Basic command-line interpreter. See nocli.h for how to use it!
//
#define _POSIX_SOURCE 1

#include "nocli.h"

#include <string.h>

// 1. <DONE, static array> internal buffers mgment? (pool v byte alloc)
// 2. <DONE> feed fxn; incoming data stream
// 3. <DONE> register command and callback table
// 4. dynamic echo configuration

#define NOCLI_HELP_COMMAND_STRING "?"
#define NOCLI_PRINTABLE_CHAR(c) ((c > 31) && (c < 127))

#define ARRAY_SIZE(obj_) (sizeof(obj_) / sizeof(*obj_))

struct NocliPrivCtx {
  // Active command buffer
  char buffer[NOCLI_CONFIG_MAX_COMMAND_LENGTH];
};
// Size test below.. there's a better way I think
// char boom[NOCLI_PRIVATE_CONTEXT_SIZE] = {[sizeof(struct NocliPrivCtx) - 1] =
// 0};

#if !defined(strnlen)
size_t strnlen(const char *str, size_t maxlen) {
  const char *cp;

  for (cp = str; maxlen != 0 && *cp != '\0'; cp++, maxlen--)
    ;

  return (size_t)(cp - str);
}
#endif

#if 0 // TODO use strsep // !defined(strsep)
char *
strsep(stringp, delim)
    char **stringp;
    const char *delim;
{
    char *s;
    const char *spanp;
    int c, sc;
    char *tok;

    if ((s = *stringp) == NULL)
        return (NULL);
    for (tok = s;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}
#endif

// Reset active buffer and print configured prompt
static void PromptReset(struct Nocli *nocli) {
  struct NocliPrivCtx *ctx = (struct NocliPrivCtx *)(nocli->private);
  ctx->buffer[0] = '\0';

  nocli->output_stream(NOCLI_CONFIG_ENDLINE_STRING,
                       sizeof(NOCLI_CONFIG_ENDLINE_STRING) - 1);
  nocli->output_stream((nocli->prefix_string),
                       strnlen(nocli->prefix_string, 1024));
}

#if NOCLI_CONFIG_HELP_COMMAND
static void PrintHelp(struct Nocli *nocli) {
#define NOCLI_PRINT_COMMAND(name)                                              \
  nocli->output_stream(NOCLI_CONFIG_ENDLINE_STRING,                            \
                       sizeof(NOCLI_CONFIG_ENDLINE_STRING) - 1);               \
  nocli->output_stream((char *)name, strnlen(name, 1024));

  NOCLI_PRINT_COMMAND("?");
  NOCLI_PRINT_COMMAND("help");

  for (size_t i = 0; i < nocli->command_table_length; i++) {
    NOCLI_PRINT_COMMAND(nocli->command_table[i].name);
    nocli->output_stream("\t", strlen("\t"));
    nocli->output_stream((char *)nocli->command_table[i].help,
                         strnlen(nocli->command_table[i].help, 1024));
  }
}
#endif // NOCLI_HELP_COMMAND

static void ProcessCommand(struct Nocli *nocli, char *command) {
  char *argv[NOCLI_CONFIG_MAX_COMMAND_TOKENS];
  size_t argc = 0, i = 0;

  // tokenize
  // TODO handle arguments enclosed in quotes, and escaped quotes
  char *token;
  char *rest = command;

  while (argc < ARRAY_SIZE(argv) && (token = strtok_r(rest, " ", &rest))) {
    argv[argc++] = token;
  }

// valid command?
#if NOCLI_CONFIG_HELP_COMMAND
  if ((strcmp("?", argv[0]) == 0) || (strcmp("help", argv[0]) == 0)) {
    PrintHelp(nocli);
  } else
#endif
  {
    for (i = 0; i < nocli->command_table_length; i++) {
      if (strcmp(nocli->command_table[i].name, argv[0]) == 0) {
        // call it
        nocli->command_table[i].function(argc, argv);
        break;
      }
    }
  }

  // command not found, emit error
  if ((i > 0) && (i == nocli->command_table_length)) {
    nocli->output_stream(NOCLI_CONFIG_ENDLINE_STRING,
                         sizeof(NOCLI_CONFIG_ENDLINE_STRING) - 1);
    nocli->output_stream(nocli->error_string,
                         strnlen(nocli->error_string, 1024));
  }
}

void Nocli_Init(struct Nocli *nocli) { PromptReset(nocli); }

void Nocli_Feed(struct Nocli *nocli, char *input, size_t length) {
  if (length == 0) {
    return;
  }

  struct NocliPrivCtx *ctx = (struct NocliPrivCtx *)(nocli->private);
  char *buffer = ctx->buffer;

  // process incoming data
  const size_t buffer_size = sizeof(ctx->buffer);
  size_t buffer_used = strnlen(buffer, 1024);
  size_t buffer_space = buffer_size - 1 - buffer_used;
  while (length > 0) {
    bool echo = false;

    switch (*input) {
    case '\n':
    case '\r':
      // line end reached, process command
      if (buffer_used > 0) {
        ProcessCommand(nocli, buffer);
      }
      PromptReset(nocli);
      buffer_used = 0;
      buffer_space = buffer_size - 1;
      break;

    case '\b':
    // some terminals will map backspace to delete, 127
    case 127:
      // backspace decrements to prompt
      if (buffer_used > 0) {
        echo = true;
        buffer[--buffer_used] = '\0';
        buffer_space++;
      }
      break;

    default:
      if (NOCLI_PRINTABLE_CHAR(*input) && (buffer_space > 0)) {
        echo = true;
        buffer[buffer_used++] = *input;
        buffer[buffer_used] = '\0';
        buffer_space--;
      }
      break;
    }

    // echo is enabled and this byte should be echoed
    if (echo && nocli->echo_on) {
      nocli->output_stream(input, 1);
    }

    // advance to next input byte
    input++;
    length--;
  }
}
