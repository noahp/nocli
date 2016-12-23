#include "nocli.h"

// 1. internal buffers mgment? (pool v byte alloc)
// 2. feed fxn; incoming data stream
// 3. register command and callback table
// 4. dynamic echo configuration
// 5. prompt prefix string on the fly

// Setup context
enum NocliErrors Nocli_Setup(struct *nocli){
    (void)nocli;
    
    return kNocliOK;
}

// Feed data in
enum NocliErrors Nocli_Feed(struct *nocli, char *input, size_t length){
    (void)nocli, (void)input, (void)length;
    
    return kNocliOK;
}
