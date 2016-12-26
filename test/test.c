//
// test.c
//
// Run some tests on nocli.
//
#include "nocli.h"
#include <string.h>
#include <stdio.h>

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
    #define ERROR_EXIT \
        printf(">> ERROR in test_nocli_prompt line %d\n", __LINE__);\
        return -1
    // setup
    mock_output_buffer_idx = 0;
    #define PROMPT_1_STRING "nocli$ "
    #define PROMPT_2_STRING "prompty!"
    static struct Nocli nocli_ctx = {
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

int main(int argc, char **argv){
    (void)argc, (void)argv;
    
    int result = test_nocli_prompt();
    if(result != 0){
        return result;
    }

    return 0;
}