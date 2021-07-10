#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// " one   two  " -> "one", "two"

// The below check works only if c_ is signed char; 127++ = -128, which is < 31
#define NOCLI_PRINTABLE_CHAR(c_) (c_ > 31)

#if defined(DEBUG)
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

int argsplit(char *buffer, size_t argv_count, char *argv[argv_count]) {
  char **const argv_start = argv;
  int argc = 0;
  int token_started = 0;
  char open_quote = 0;
  while (*buffer != '\0') {
    DEBUG_PRINTF(">> c '%c'\n", *buffer);
    if (!NOCLI_PRINTABLE_CHAR(*buffer)) {
      // unsupported character :(
      return 0;
    }

    switch (*buffer) {
    case ' ':
      if (!open_quote && token_started) {
        *buffer = '\0';
        argc++;
        token_started = 0;
      } else if (open_quote && !token_started) {
        // first character after quote open is ' ', start token
        if (argv - argv_start == argv_count) {
          // no more space for tokens
          goto done;
        }
        *argv++ = buffer;
        token_started = 1;
      }
      break;

    case '\'':
    case '"':
      if (open_quote == *buffer) {
        // current quote closes, slide remaining string one character to the
        // left, to elide the quote
        DEBUG_PRINTF(">> close quote '%c'\n", *buffer);
        open_quote = 0;
        memmove(buffer, buffer + 1, strlen(buffer) + 1);
        buffer--;
      } else if (!open_quote) {
        DEBUG_PRINTF(">> open quote '%c'\n", *buffer);
        open_quote = *buffer;
      } else if (!token_started) {
        // character after an open quote started token is the other quote type
        if (argv - argv_start == argv_count) {
          // no more space for tokens
          goto done;
        }
        *argv++ = buffer;
        token_started = 1;
      }
      break;

    default:
      if (!token_started) {
        // store next token start
        if (argv - argv_start == argv_count) {
          // no more space for tokens
          goto done;
        }
        *argv++ = buffer;
        token_started = 1;
      }

      break;
    }
    buffer++;
  }

done:
  if (token_started) {
    argc++;
  }

  return argc;
}

int main(int argc_outer, char **argv_outer) {

  struct testvect {
    char buffer[128];
    int argc;
    char *argv[10];
  } testvect[] = {
      {
          .buffer = "zero one",
          .argc = 2,
          .argv =
              {
                  "zero",
                  "one",
              },
      },
      {
          .buffer = "   zero  one two   ",
          .argc = 3,
          .argv =
              {
                  "zero",
                  "one",
                  "two",
              },
      },
      {
          .buffer = "\"zero  \" '\" one\"\"'  ",
          .argc = 2,
          .argv =
              {
                  "zero  ",
                  "\" one\"\"",
              },
      },
      {
          .buffer = "0 1 2 3 4 5 6 7 8 9 10 11",
          .argc = 10,
          .argv =
              {
                  "0",
                  "1",
                  "2",
                  "3",
                  "4",
                  "5",
                  "6",
                  "7",
                  "8",
                  "9",
              },
      },
  };

  for (size_t i = 0; i < (sizeof(testvect) / sizeof(*testvect)); i++) {
    char *argv[10] = {0};
    printf(">>test %zu\n", i);

    int argc = argsplit(testvect[i].buffer, sizeof(argv) / sizeof(*argv), argv);

    char **pargv = &argv[0];
    int mismatch = 0;
    if (argc != testvect[i].argc) {
      printf("wrong arg count, %d expected %d got\n", testvect[i].argc, argc);
      mismatch = 1;
    }
    for (size_t j = 0; j < argc; j++) {
      printf("%s\n", *pargv++);
      if (strcmp(testvect[i].argv[j], argv[j]) != 0) {
        printf(">>mismatch: '%s' vs '%s'\n", testvect[i].argv[j], argv[j]);
      }
    }
    if (mismatch) {
      printf(">>fail!\n");
      exit(-1);
    }
  }

  return 0;
}
