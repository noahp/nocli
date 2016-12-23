#include <stdio.h>
#include "../src/nocli.h"

void function1(int argc, char **argv){
    (void)argv;
    printf("[function1] %d\n", argc);
}

void function2(int argc, char **argv){
    (void)argv;
    printf("[function2] %d\n", argc);
}

void output(void *data, size_t length){
    printf("%.*s", (int)length, (char*)data);
}

int main(int argc, char **argv){
    (void)argc, (void)argv;
    char ch;
    
    // setup context
    struct NocliCommand cmdlist[] ={
        {
            .name = "function1",
            .function = function1,
        },
        {
            .name = "function2",
            .function = function2,
        },
    };
    struct Nocli nocli_ctx = {
        .output_stream = output,
        .command_table = cmdlist,
        .command_table_length = sizeof(cmdlist)/sizeof(cmdlist[0]),
        .prefix_string = "nocli $ ",
    };
    
    if(Nocli_Init(&nocli_ctx)){
        return -1;
    }
    
    // feed characters from getchar. turn off buffering first, and capture ctrl-c
    while((ch = getchar()) != 3){
        Nocli_Feed(&nocli_ctx, &ch, sizeof(ch));
    }    
    
    return 0;
}