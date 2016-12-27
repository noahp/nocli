//
// nocli.c
//
// Basic command-line interpreter. See nocli.h for how to use it!
//
#include <string.h>
#include "nocli.h"

// 1. <DONE, static array> internal buffers mgment? (pool v byte alloc)
// 2. <DONE> feed fxn; incoming data stream
// 3. <DONE> register command and callback table
// 4. dynamic echo configuration

#define NOCLI_HELP_COMMAND_STRING "?"

struct NocliPrivCtx {
    // History buffer is singley-linked-list
    struct History {
        struct History *next;
        char buffer[NOCLI_CONFIG_MAX_COMMAND_LENGTH];
    } history[NOCLI_CONFIG_HISTORY_DEPTH];
    
    // Active command buffer
    char buffer[NOCLI_CONFIG_MAX_COMMAND_LENGTH];
};
// Size test below.. there's a better way I think
//char boom[NOCLI_PRIVATE_CONTEXT_SIZE] = {[sizeof(struct NocliPrivCtx) - 1] = 0};

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

#if !defined(strncmp)
int
strncmp(const char *s1, const char *s2, size_t n)
{
    for ( ; n > 0; s1++, s2++, --n)
	if (*s1 != *s2)
	    return ((*(unsigned char *)s1 < *(unsigned char *)s2) ? -1 : +1);
	else if (*s1 == '\0')
	    return 0;
    return 0;
}
#endif

#if 0// TODO use strsep // !defined(strsep)
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
    
    nocli->output_stream("\n", 1);
    nocli->output_stream((nocli->prefix_string), strnlen(nocli->prefix_string, 1024));
}

#if NOCLI_CONFIG_HELP_COMMAND
static void PrintHelp(struct Nocli *nocli){
    #define NOCLI_PRINT_COMMAND(name) nocli->output_stream("\n", 1);\
        nocli->output_stream((char*)name, strnlen(name, 1024));
    
    NOCLI_PRINT_COMMAND("?");
    NOCLI_PRINT_COMMAND("help");
    
    for(size_t i=0; i<nocli->command_table_length; i++){
        NOCLI_PRINT_COMMAND(nocli->command_table[i].name);
    }
}
#endif // NOCLI_HELP_COMMAND

static void ProcessCommand(struct Nocli *nocli, char *command){
    char *argv[NOCLI_CONFIG_MAX_COMMAND_TOKENS];
    size_t argc = 0, i = 0;
    
    // tokenize
    // TODO handle arguments enclosed in quotes, and escaped quotes
    argv[argc] = strtok(command, " ");
    while((argc < (int)(sizeof(argv)/sizeof(argv[0]))) && (argv[argc] != NULL)){
        argc++;
        argv[argc] = strtok(NULL, " ");
    }

    // valid command?
    #if NOCLI_CONFIG_HELP_COMMAND
    if((strncmp("?", argv[0], 1024) == 0) ||
       (strncmp("help", argv[0], 1024) == 0)){
        PrintHelp(nocli);
    }
    else
    #endif
    {
        for(i=0; i<nocli->command_table_length; i++){
            if(strncmp(nocli->command_table[i].name, argv[0], 1024) == 0){
                // call it
                nocli->command_table[i].function(argc, argv);
                break;
            }
        }
    }
    
    // command not found, emit error
    if((i > 0) && (i == nocli->command_table_length)){
        nocli->output_stream("\n", 1);
        nocli->output_stream(nocli->error_string, strnlen(nocli->error_string, 1024));
    }
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
    while(length > 0){
        bool echo = false;

        switch(*input){
            case NOCLI_CONFIG_ENDLINE:
                // line end reached, process command
                if(buffer_used > 0){
                    ProcessCommand(nocli, ctx->buffer);
                }
                PromptReset(nocli);
                buffer_used = 0;
                buffer_space = sizeof(ctx->buffer) - 1;
                break;
                
            case '\b':
                // backspace decrements to prompt
                if(buffer_used > 0){
                    echo = true;
                    ctx->buffer[--buffer_used] = '\0';
                    buffer_space++;
                }
                break;
                
            default:
                if(buffer_space > 0){
                    echo = true;
                    ctx->buffer[buffer_used++] = *input;
                    ctx->buffer[buffer_used] = '\0';
                    buffer_space--;
                }
                break;
        }
        
        // echo is enabled and this byte should be echoed
        if(echo && nocli->echo_on){
            nocli->output_stream(input, 1);
        }

        // advance to next input byte
        input++;
        length--;
    }

    return kNocliOK;
}
