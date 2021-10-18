# no-cli

[![GitHub](https://img.shields.io/badge/GitHub-noahp%2Fnocli-8da0cb?style=for-the-badge&logo=github)](https://github.com/noahp/nocli)

[![Build Status](https://img.shields.io/travis/noahp/nocli.svg?style=for-the-badge)](https://travis-ci.org/noahp/nocli)
[![Codecov](https://img.shields.io/codecov/c/github/noahp/nocli.svg?style=for-the-badge)](https://codecov.io/gh/noahp/nocli)
[![C99](https://img.shields.io/badge/language-C99-blue.svg?style=for-the-badge)](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf)

Cheesy command line interpreter designed for tiny memory-constrained systems.
Minimal dependencies: just string.h.

## Features

- low footprint; the library is ~**600 bytes** `.text` (~**335 bytes** under
  minimal configuration), and uses a (configurable) 128 byte buffer (`.bss`) and
  24 byte (`.data`) context structure.

  ```bash
  ❯ arm-none-eabi-gcc -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -Os -Itest -c nocli.c -o nocli.o
  ❯ arm-none-eabi-size nocli.o
     text    data     bss     dec     hex filename
      582       0       0     582     246 nocli.o
  ```

- incremental parsing of input stream
- static memory usage
- supports doubly- and singly- quoted args (can be disabled to save ~100 bytes)
- 100% test coverage
- fuzz tested via LibFuzzer

Possible future features, if I need them:

- UNIMPLEMENTED optional tab completion or history?

## Usage

### Example

See [`test/example.c`](test/example.c) for an example that runs on host.

You can try the example by building it and running it:

```bash
# build
❯ make -f test/Makefile
Compiling nocli.c
Compiling test/example.c
Linking build/example/example

# run
❯ ./build/example/example

nocli$ help

?
help
count-args      print number of args passed
change-prompt   set prompt to new string
nocli$ count-args 1 2 3
Arg count: 3

nocli$
```

### Integration

API is documented in [`nocli.h`](nocli.h). There's only two functions used:

1. initialize the library: `void Nocli_Init(struct Nocli *nocli);`
   > this includes a callback for outputting data and the table of commands.
2. pass any amount of data: `void Nocli_Feed(struct Nocli *nocli, const char *input, size_t length);`
   > commands are executed within this function, so be sure to call it in a safe context

## libfuzzer crashes

Run libfuzzer with `make -f test/Makefile fuzz`; it will run continuously until
a crash occurs (`AddressSanitizer` and `UndefinedBehaviorSanitizer` are enabled
on the fuzz build)

Crashes that libfuzzer finds are saved in [test/corpus](test/corpus), which are
run through the library in ci to catch regressions.

## License

WTFPL (http://www.wtfpl.net/) or public domain, whichever you prefer.
