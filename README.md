# no-cli

[![GitHub](https://img.shields.io/badge/GitHub-noahp%2Fnocli-8da0cb?style=for-the-badge&logo=github)](https://github.com/noahp/nocli)


[![Build Status](https://img.shields.io/travis/noahp/nocli.svg?style=for-the-badge)](https://travis-ci.org/noahp/nocli)
[![Codecov](https://img.shields.io/codecov/c/github/noahp/nocli.svg?style=for-the-badge)](https://codecov.io/gh/noahp/nocli)
[![C99](https://img.shields.io/badge/language-C99-blue.svg?style=for-the-badge)](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf)

Cheesy command line interpreter designed for tiny memory-constrained systems.
Minimal dependencies: just string.h.

## Features

* tiny footprint; the library is ~**600 bytes** `.text` (~**335 bytes** under
  minimal configuration), and uses a (configurable) 128 byte buffer (`.bss`) and
  24 byte (`.data`) context structure. a minimal integration could be similar to
  the example application, which clocks in at:

   ```bash
    ❯ make -f test/Makefile_cortexm4.mk
   Compiling src/nocli.c
   Compiling test/example.c
   Linking build/libnocli_example.a
      text    data     bss     dec     hex filename
      582       0       0     582     246 nocli.o (ex build/libnocli_example.a)
      310      24     153     487     1e7 example.o (ex build/libnocli_example.a)
      892      24     153    1069     42d (TOTALS)
   ```

* incremental parsing of input stream
* static memory usage
* TODO optional tab completion or history?

## License

WTFPL (http://www.wtfpl.net/) or public domain, whichever you prefer.
