#include <stdio.h>
#include "../src/nocli.h"

int main(int argc, char **argv){
    (void) argc;
    (void) argv;
    
    struct Nocli;
    
    if(Nocli_Setup(&Nocli)){
        return -1;
    }
    
    return 0;
}