//
// nocli.c
// 
// Basic command-line interpreter. See nocli.h for how to use it!
// 
#include <string.h>
#include "nocli.h"

// 1. internal buffers mgment? (pool v byte alloc)
// 2. feed fxn; incoming data stream
// 3. register command and callback table
// 4. dynamic echo configuration
// 5. prompt prefix string on the fly

#define NOCLI_HELP_COMMAND_STRING "?"

struct NocliPrivCtx {
    // History buffer is singley-linked-list
    struct History {
        struct History *next;
        char buffer[NOCLI_MAX_COMMAND_LENGTH];
    } history[NOCLI_HISTORY_DEPTH];
    
    // Active command buffer
    char buffer[NOCLI_MAX_COMMAND_LENGTH];
};
// Size test below.. there's a better way I think
//char boom[NOCLI_PRIVATE_CONTEXT_SIZE] = {[sizeof(struct NocliPrivCtx) - 1] = 0};

// TODO sub in a custom strnlen
#if !defined(strnlen)
size_t
strnlen(const char *str, size_t maxlen)
{
	const char *cp;

	for (cp = str; maxlen != 0 && *cp != '\0'; cp++, maxlen--)
		;

	return (size_t)(cp - str);
}
#endif

#if !defined(strsep)
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
static void PromptReset(struct Nocli *nocli){
    struct NocliPrivCtx *ctx = (struct NocliPrivCtx *)(nocli->private);
    ctx->buffer[0] = '\0';
    
    nocli->output_stream((void*)"\n", 1);
    nocli->output_stream((void*)(nocli->prefix_string), strnlen(nocli->prefix_string, 1024));
}

static void ProcessCommand(struct Nocli *nocli, char *command){
    char *argv[NOCLI_MAX_COMMAND_TOKENS];
    size_t argc = 0;
    
    // tokenize
    // TODO handle arguments enclosed in quotes, and escaped quotes
    argv[argc] = strtok(command, " ");
    while((argc < (int)(sizeof(argv)/sizeof(argv[0]))) && (argv[argc] != NULL)){
        printf("%s ", argv[argc]);
        argc++;
        argv[argc] = strtok(NULL, " ");
    }

    // TODO valid command?
    (void)nocli;
    // TODO call it
}

enum NocliErrors Nocli_Init(struct Nocli *nocli){
    PromptReset(nocli);
        
    return kNocliOK;
}

enum NocliErrors Nocli_Feed(struct Nocli *nocli, char *input, size_t length){    
    if(length == 0){
        return kNocliOK;
    }
    struct NocliPrivCtx *ctx = (struct NocliPrivCtx *)(nocli->private);
    
    // process incoming data
    size_t buffer_used = strnlen(ctx->buffer, 1024);
    size_t buffer_space = sizeof(ctx->buffer) - 1 - buffer_used;
    while((length > 0) && (buffer_space > 0)){
        // if line end reached, process command
        if(*input == NOCLI_ENDLINE){
            ProcessCommand(nocli, ctx->buffer);
            PromptReset(nocli);
            buffer_used = 0;
            buffer_space = sizeof(ctx->buffer) - 1;
            input++;
        }
        else{
            ctx->buffer[buffer_used++] = *input++;
            ctx->buffer[buffer_used] = '\0';
            buffer_space--;
        }

        length--;
    }

    return kNocliOK;
}
