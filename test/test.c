//
// test.c
//
// Run some tests on nocli.
//
#include "nocli.h"
#include <string.h>
#include <stdio.h>

#define ERROR_EXIT \
    printf(">> ERROR at line %d\n", __LINE__);\
    return -1

#if !defined(MIN)
#define MIN(a, b) ((a) < (b))?(a):(b)
#endif
static size_t mock_output_buffer_idx = 0;
static char mock_output_buffer[1024];
static void mock_output(char *data, size_t length){
    length = MIN(length, sizeof(mock_output_buffer)-mock_output_buffer_idx);
    memcpy(&mock_output_buffer[mock_output_buffer_idx], data, length);
    mock_output_buffer_idx += length;
}

int test_nocli_prompt(void){
    // setup
    mock_output_buffer_idx = 0;
    #define PROMPT_1_STRING "nocli$ "
    #define PROMPT_2_STRING "prompty!"
    struct Nocli nocli_ctx = {
        .output_stream = mock_output,
        .command_table = NULL,
        .command_table_length = 0,
        .prefix_string = PROMPT_1_STRING,
        .error_string = "error, command not found",
        .echo_on = true,
    };
    
    if(Nocli_Init(&nocli_ctx)){
        ERROR_EXIT;
    }
    
    // check result
    if(mock_output_buffer_idx != sizeof("\n" PROMPT_1_STRING) - 1){
        ERROR_EXIT;
    }
    if(memcmp(mock_output_buffer, "\n" PROMPT_1_STRING, mock_output_buffer_idx) != 0){
        printf("%.*s\n", (int)mock_output_buffer_idx, mock_output_buffer);
        ERROR_EXIT;
    }

    // modify prompt text, and feed a newline
    mock_output_buffer_idx = 0;
    nocli_ctx.prefix_string = PROMPT_2_STRING;
    if(Nocli_Feed(&nocli_ctx, "\n", 1)){
        ERROR_EXIT;
    }
    if(mock_output_buffer_idx != sizeof("\n" PROMPT_2_STRING) - 1){
        ERROR_EXIT;
    }
    if(memcmp(mock_output_buffer, "\n" PROMPT_2_STRING, mock_output_buffer_idx) != 0){
        printf("%.*s\n", (int)mock_output_buffer_idx, mock_output_buffer);
        ERROR_EXIT;
    }

    return 0;
}

static bool function1_iscalled = false;
static void function1(int argc, char** argv){
    (void)argc, (void)argv;
    function1_iscalled = true;
}

static int test_command_call(void){
    // setup
    mock_output_buffer_idx = 0;
    function1_iscalled = false;
    struct NocliCommand commands[] = {
        {
            .name = "function1",
            .function = function1,
        }
    };
    struct Nocli nocli_ctx = {
        .output_stream = mock_output,
        .command_table = commands,
        .command_table_length = sizeof(commands)/sizeof(commands[0]),
        .prefix_string = "nocli $",
        .error_string = "error, command not found",
        .echo_on = true,
    };

    if(Nocli_Init(&nocli_ctx)){
        ERROR_EXIT;
    }

    // feed invalid command, check output
    #define BAD_COMMAND_STRING "badcommand arg1 arg2\n"
    if(Nocli_Feed(&nocli_ctx, BAD_COMMAND_STRING, sizeof(BAD_COMMAND_STRING) - 1)){
        ERROR_EXIT;
    }
    if(mock_output_buffer_idx != sizeof("\nnocli $" BAD_COMMAND_STRING "error, command not found\nnocli $") - 1){
        ERROR_EXIT;
    }
    if(memcmp(mock_output_buffer, "\nnocli $" BAD_COMMAND_STRING "error, command not found\nnocli $",
           mock_output_buffer_idx) != 0){
        ERROR_EXIT;
    }
    
    // valid command, check output and command executed
    mock_output_buffer_idx = 0;
    if(Nocli_Feed(&nocli_ctx, "function1 arg1\n", sizeof("function1 arg1\n"))){
        ERROR_EXIT;
    }
    if(mock_output_buffer_idx != sizeof("function1 arg1\nnocli $")){
        ERROR_EXIT;
    }
    if(memcmp(mock_output_buffer, "function1 arg1\nnocli $", mock_output_buffer_idx) != 0){
        ERROR_EXIT;
    }
    if(function1_iscalled == false){
        ERROR_EXIT;
    }

    return 0;
}

static int test_help(void){
    // setup
    mock_output_buffer_idx = 0;
    struct NocliCommand commands[] = {
        {
            .name = "function1",
            .function = function1,
        }
    };
    struct Nocli nocli_ctx = {
        .output_stream = mock_output,
        .command_table = commands,
        .command_table_length = sizeof(commands)/sizeof(commands[0]),
        .prefix_string = "nocli $",
        .error_string = "error, command not found",
        .echo_on = true,
    };

    if(Nocli_Init(&nocli_ctx)){
        ERROR_EXIT;
    }
    // test entering '?' command, with multiple backspaces; we should only get backspaces until the
    // active buffer is blank
    if(Nocli_Feed(&nocli_ctx, "?\b\b?\n", sizeof("?\b\b?\n") - 1)){
        ERROR_EXIT;
    }

    // check result- help string
    if(mock_output_buffer_idx != sizeof("\nnocli $?\b?\n?\nhelp\nfunction1\nnocli $") - 1){
        printf("%d %.*s\n", (int)mock_output_buffer_idx, (int)mock_output_buffer_idx, mock_output_buffer);
        ERROR_EXIT;
    }
    if(memcmp(mock_output_buffer, "\nnocli $?\b?\n?\nhelp\nfunction1\nnocli $", mock_output_buffer_idx) != 0){
        printf("%.*s\n", (int)mock_output_buffer_idx, mock_output_buffer);
        ERROR_EXIT;
    }

    return 0;
}

int main(int argc, char **argv){
    (void)argc, (void)argv;

    return (test_nocli_prompt() ||
            test_command_call() ||
            test_help())?(-1):(0);
}