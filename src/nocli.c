//
// nocli.c
//
// Basic command-line interpreter. See nocli.h for how to use it!
//

#include "nocli.h"

#include <string.h>

#define NOCLI_COMMAND_NOT_FOUND_STRING "error, command not found"

// The below check works only if c_ is signed char; 127++ = -128, which is < 31
#define NOCLI_PRINTABLE_CHAR(c_) (c_ > 31)
#define ARRAY_SIZE(obj_) (sizeof(obj_) / sizeof(*obj_))

#if defined(DEBUG)
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

struct NocliPrivCtx {
	// Active command buffer
	char buffer[NOCLI_CONFIG_MAX_COMMAND_LENGTH];
};
// Size test below.. there's a better way I think
// char boom[NOCLI_PRIVATE_CONTEXT_SIZE] = {[sizeof(struct NocliPrivCtx) - 1] =
// 0};

// Reset active buffer and print configured prompt
static void PromptReset(struct Nocli *nocli) {
	struct NocliPrivCtx *ctx = (struct NocliPrivCtx *)(nocli->private);
	ctx->buffer[0] = '\0';

	nocli->output_stream(
		NOCLI_CONFIG_ENDLINE_STRING, sizeof(NOCLI_CONFIG_ENDLINE_STRING) - 1);
	nocli->output_stream((nocli->prefix_string), strlen(nocli->prefix_string));
}

#if NOCLI_CONFIG_HELP_COMMAND
static void PrintHelp(struct Nocli *nocli) {
	nocli->output_stream(
		NOCLI_CONFIG_ENDLINE_STRING "?" NOCLI_CONFIG_ENDLINE_STRING "help",
		strlen(NOCLI_CONFIG_ENDLINE_STRING "?" NOCLI_CONFIG_ENDLINE_STRING
										   "help"));

	for (size_t i = 0; i < nocli->command_table_length; i++) {
		nocli->output_stream(
			NOCLI_CONFIG_ENDLINE_STRING,
			sizeof(NOCLI_CONFIG_ENDLINE_STRING) - 1);
		nocli->output_stream(
			nocli->command_table[i].name, strlen(nocli->command_table[i].name));

		nocli->output_stream("\t", strlen("\t"));
		nocli->output_stream(
			nocli->command_table[i].help, strlen(nocli->command_table[i].help));
	}
}
#endif  // NOCLI_HELP_COMMAND

NOCLI_ATTRIBUTE_ACCESS(write_only, 3, 2)
static int argsplit(char *buffer, int argv_count, char **argv) {
#if NOCLI_QUOTED_ARGS_SUPPORT
	char **const argv_start = argv;
	int argc = 0;
	int token_started = 0;
	char open_quote = 0;
	while (*buffer != '\0') {
		DEBUG_PRINTF(">> c '%c'\n", *buffer);

		switch (*buffer) {
			case ' ':
				if (!open_quote && token_started) {
					*buffer = '\0';
					argc++;
					token_started = 0;
				} else if (open_quote && !token_started) {
					// first character after quote open is ' ', start token
					if ((argv - argv_start) == argv_count) {
						// no more space for tokens
						goto done;
					}
					*argv++ = buffer;
					token_started = 1;
				}
				break;

			case '\'':
			case '"':
				if (open_quote == *buffer) {
					// current quote closes, slide remaining string one
					// character to the left, to elide the quote
					DEBUG_PRINTF(">> close quote '%c'\n", *buffer);
					open_quote = 0;
					memmove(buffer, buffer + 1, strlen(buffer));
					buffer--;
				} else if (!open_quote) {
					DEBUG_PRINTF(">> open quote '%c'\n", *buffer);
					open_quote = *buffer;
				} else if (!token_started) {
					// character after an open quote started token is the other
					// quote type
					if ((argv - argv_start) == argv_count) {
						// no more space for tokens
						goto done;
					}
					*argv++ = buffer;
					token_started = 1;
				}
				break;

			default:
				if (!token_started) {
					// store next token start
					if ((argv - argv_start) == argv_count) {
						// no more space for tokens
						goto done;
					}
					*argv++ = buffer;
					token_started = 1;
				}

				break;
		}
		buffer++;
	}

done:
	if (token_started) {
		argc++;
	}

	return argc;
}
#else  // NOCLI_QUOTED_ARGS_SUPPORT
	char **const argv_start = argv;
	int argc = 0;
	int in_token = 0;
	while (((argv - argv_start) < argv_count) && (*buffer != '\0')) {
		if (*buffer == ' ') {
			if (in_token) {
				*buffer = '\0';
				in_token = 0;
			}
		} else {
			if (!in_token) {
				argc++;
				*argv++ = buffer;
				in_token = 1;
			}
		}

		buffer++;
	}

	return argc;
}
#endif

static void ProcessCommand(struct Nocli *nocli, char *command) {
	char *argv[NOCLI_CONFIG_MAX_COMMAND_TOKENS];
	argv[0] = "";

	// tokenize
	int argc = argsplit(command, ARRAY_SIZE(argv), argv);

// valid command?
#if NOCLI_CONFIG_HELP_COMMAND
	if ((strcmp("?", argv[0]) == 0) || (strcmp("help", argv[0]) == 0)) {
		PrintHelp(nocli);
	} else
#endif
	{
		for (size_t i = 0; i < nocli->command_table_length; i++) {
			if (strcmp(nocli->command_table[i].name, argv[0]) == 0) {
				// call it
				nocli->command_table[i].function((int)argc, argv);
				return;
			}
		}

		// command not found, emit error
		if (nocli->command_table_length > 0) {
			nocli->output_stream(
				NOCLI_CONFIG_ENDLINE_STRING NOCLI_COMMAND_NOT_FOUND_STRING,
				strlen(NOCLI_CONFIG_ENDLINE_STRING
						   NOCLI_COMMAND_NOT_FOUND_STRING));
		}
	}
}

void Nocli_Init(struct Nocli *nocli) { PromptReset(nocli); }

NOCLI_ATTRIBUTE_ACCESS(read_only, 2, 3)
void Nocli_Feed(struct Nocli *nocli, const char *input, size_t length) {
	struct NocliPrivCtx *ctx = (struct NocliPrivCtx *)(nocli->private);
	char *const buffer = ctx->buffer;
	char *buffer_ptr = buffer + strlen(buffer);
	char *const buffer_end = ctx->buffer + sizeof(ctx->buffer) - 1;

	// process incoming data
	while (length > 0) {
		bool echo = false;

		char c = *input;
		if ((c == '\n') || (c == '\r')) {
			// line end reached, process command
			if (buffer_ptr > buffer) {
				ProcessCommand(nocli, buffer);
			}
			PromptReset(nocli);
			buffer_ptr = buffer;
		}
		// some terminals will map backspace to delete, 127
		else if ((c == '\b') || (c == 127)) {
			// backspace decrements to prompt
			if (buffer_ptr > buffer) {
				echo = true;
				*buffer_ptr-- = '\0';
			}
		} else {
			// drop remaining characters if we're at the limit of what we can
			// buffer
			if (NOCLI_PRINTABLE_CHAR(c) && (buffer_ptr < buffer_end)) {
				echo = true;
				*buffer_ptr++ = *input;
				*buffer_ptr = '\0';
			}
		}

		// echo is enabled and this byte should be echoed
		if (echo && nocli->echo_on) {
			nocli->output_stream(input, 1);
		}

		// advance to next input byte
		input++;
		length--;
	}
}
