#include <stdint.h>
#include <stddef.h>

enum NocliErrors {
    kNocliOK = 0,
    kNocliError,
    kNocliOutOfMemory,
};

struct Nocli {
    const char *prefix_string;
};

// Setup context
enum NocliErrors Nocli_Setup(struct Nocli *nocli);

// Feed data in
enum NocliErrors Nocli_Feed(struct Nocli *nocli, char *input, size_t length);
