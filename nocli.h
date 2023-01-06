//
// nocli.h
//
// Basic command-line interpreter.
//
// # Usage
// Instantiate and fill in required sections of `struct Nocli`; see `example.c`
// for a simple example.
//
// # Configuration
// Buffer sizes, etc. can be adjusted by defining `NOCLI_CONFIG_*` options in
// `nocli_config.h`.
//
#pragma once
#include "nocli_config.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Maximum command length
#if !defined(NOCLI_CONFIG_MAX_COMMAND_LENGTH)
#define NOCLI_CONFIG_MAX_COMMAND_LENGTH (128)
#endif

// Maximum number of tokens in a single command (eg "command option1 option2" is
// 3 tokens)
#if !defined(NOCLI_CONFIG_MAX_COMMAND_TOKENS)
#define NOCLI_CONFIG_MAX_COMMAND_TOKENS (10)
#endif

// Commands are always terminated by LF, '\n'. Output line end can be configured
// here
#if !defined(NOCLI_CONFIG_ENDLINE_STRING)
#define NOCLI_CONFIG_ENDLINE_STRING ("\n")
#endif

// Built in help command, enabled by default, set to 0 to disable
#if !defined(NOCLI_CONFIG_HELP_COMMAND)
#define NOCLI_CONFIG_HELP_COMMAND (1)
#endif

// Support for quoted args, eg "arg with spaces" etc.
// Adds approximately 80 bytes
#if !defined(NOCLI_QUOTED_ARGS_SUPPORT)
#define NOCLI_QUOTED_ARGS_SUPPORT (1)
#endif

// Disable runtime echo control; echo will be always enabled
#if !defined(NOCLI_RUNTIME_ECHO_CONTROL)
#define NOCLI_RUNTIME_ECHO_CONTROL (1)
#endif

#if !defined(__clang__) && (__GNUC__ >= 11)
#define NOCLI_ATTRIBUTE_ACCESS(mode_, ref_index_, size_index_)                 \
  __attribute__((access(mode_, ref_index_, size_index_)))
#else
#define NOCLI_ATTRIBUTE_ACCESS(mode_, ref_index_, size_index_)
#endif

// Command structure
struct NocliCommand {
  const char *name;                        // command name string, eg "cd"
  void (*function)(int argc, char **argv); // command function
#if NOCLI_CONFIG_HELP_COMMAND
  const char *help; // help string
#endif
};

// Nocli context, intantiated by calling code. All the memory used by nocli
// exists here.
//  Fields are divided into 3 classes:
//  1. internally managed; obfuscated block
//  2. configured prior to setup
//  3. configured prior to setup; reconfigurable at runtime

#define NOCLI_PRIVATE_CONTEXT_SIZE NOCLI_CONFIG_MAX_COMMAND_LENGTH
struct NocliPrivate {
  uint8_t private[NOCLI_PRIVATE_CONTEXT_SIZE];
};

struct Nocli {
  // 1. private context space
  struct NocliPrivate *private;

  // 2. configured prior to setup
  void (*output_stream)(const char *, size_t); // nocli uses this for stdout

  // 3. reconfiguratble at runtime
  const struct NocliCommand *command_table; // table of commands
  size_t
      command_table_length; // length of command table; must match command table
  char *prefix_string;      // leading string for prompt (eg "$ ")
#if NOCLI_RUNTIME_ECHO_CONTROL
  bool echo_on; // enable or disable echo
#endif
};

// Initialize context.
// Must be called before calling `Nocli_Feed`. Will reset any context state and
// print prompt.
void Nocli_Init(struct Nocli *nocli);

// Feed data in.
// Passed memory may be modified by nocli, and must remain valid until this
// function returns.
void Nocli_Feed(struct Nocli *nocli, const char *input, size_t length);
