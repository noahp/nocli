#include <stdio.h>
#include "../src/nocli.h"

int main(int argc, char **argv){
    (void)argc, (void)argv;
    char ch;
    
    // setup context
    struct Nocli nocli_ctx;
    
    if(Nocli_Setup(&nocli_ctx)){
        return -1;
    }
    
    // feed characters from getchar
    while((ch = getchar()) != EOF){
        Nocli_Feed(&nocli_ctx, &ch, sizeof(ch));
    }    
    
    return 0;
}