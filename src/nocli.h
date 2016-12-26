#include <stdint.h>
#include <stddef.h>

// Depth of command history
#if !defined(NOCLI_HISTORY_DEPTH)
#define NOCLI_HISTORY_DEPTH (5)
#endif

// Maximum command length
#if !defined(NOCLI_MAX_COMMAND_LENGTH)
#define NOCLI_MAX_COMMAND_LENGTH (128)
#endif

// Maximum command tokens
#if !defined(NOCLI_MAX_COMMAND_TOKENS)
#define NOCLI_MAX_COMMAND_TOKENS (10)
#endif

// Single character endline
#if !defined(NOCLI_ENDLINE)
#define NOCLI_ENDLINE ('\n')
#endif

// Built in help command, enabled by default, set to 0 to disable
#if !defined(NOCLI_HELP_COMMAND)
#define NOCLI_HELP_COMMAND (1)
#endif

// Module error type
enum NocliErrors {
    kNocliOK = 0,
    kNocliError,
    kNocliOutOfMemory,
};

// Command structure
struct NocliCommand {
    const char *name;    // command name string, eg "cd"
    void (*function)(int argc, char **argv);    // command function
};

// Nocli context, intantiated by calling code. All the memory used by nocli exists here.
//  Fields are divided into 3 classes:
//  1. configured prior to setup
//  2. configured prior to setup; reconfigurable at runtime
//  3. internally managed; obfuscated block
#define NOCLI_PRIVATE_CONTEXT_SIZE (NOCLI_MAX_COMMAND_LENGTH +\
    (NOCLI_HISTORY_DEPTH * (NOCLI_MAX_COMMAND_LENGTH + sizeof(void*))))
struct Nocli {
    // 1. configured prior to setup
    void (*output_stream)(char *, size_t);    // nocli uses this for stdout
    const struct NocliCommand *command_table;    // table of commands
    const size_t command_table_length;    // length of command table
    
    // 2. reconfiguratble at runtime
    char *prefix_string;    // leading string for prompt (eg "$ ")
    char *error_string;    // print this if there's an error
    
    // 3. private context space
    uint8_t private[NOCLI_PRIVATE_CONTEXT_SIZE];
};

// Initialize context
enum NocliErrors Nocli_Init(struct Nocli *nocli);

// Feed data in
enum NocliErrors Nocli_Feed(struct Nocli *nocli, char *input, size_t length);
