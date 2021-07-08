# no-cli

[![GitHub](https://img.shields.io/badge/GitHub-noahp%2Fnocli-8da0cb?style=for-the-badge&logo=github)](https://github.com/noahp/nocli)


[![Build Status](https://img.shields.io/travis/noahp/nocli.svg?style=for-the-badge)](https://travis-ci.org/noahp/nocli)
[![Codecov](https://img.shields.io/codecov/c/github/noahp/nocli.svg?style=for-the-badge)](https://codecov.io/gh/noahp/nocli)
[![C99](https://img.shields.io/badge/language-C99-blue.svg?style=for-the-badge)](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf)

Cheesy command line interpreter designed for tiny memory-constrained systems.
Minimal dependencies: just string.h.

## Features

* tiny footprint; the library is ~500 bytes `.text` (~350 with help disabled),
  and uses a (configurable) 128 byte buffer (`.bss`) and 24 byte (`.data`)
  context structure. a minimal integration could be similar to the example
  application, which clocks in at:

   ```bash
    ❯ make -f test/Makefile_cortexm4.mk
    Compiling src/nocli.c
    Compiling test/example.c
    Linking build/libnocli_example.a
    text    data     bss     dec     hex filename
        511       0       0     511     1ff nocli.o (ex build/libnocli_example.a)
        283      24     128     435     1b3 example.o (ex build/libnocli_example.a)
        794      24     128     946     3b2 (TOTALS)
   ```

* incremental parsing of input stream
* static memory usage
* TODO optional tab completion

## License

WTFPL (http://www.wtfpl.net/) or public domain, whichever you prefer.
