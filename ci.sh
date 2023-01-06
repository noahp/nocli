#!/usr/bin/env bash

set -ex
set -o pipefail

# commit checker
pre-commit run --all-files

# build for cortex-m4
git clean -dxf
make -f test/Makefile_cortexm4.mk
git clean -dxf
CFLAGS="-DNOCLI_CONFIG_HELP_COMMAND=0" make -f test/Makefile_cortexm4.mk
git clean -dxf
CFLAGS="-DNOCLI_CONFIG_HELP_COMMAND=0 -DNOCLI_QUOTED_ARGS_SUPPORT=0" make -f test/Makefile_cortexm4.mk

# compilation + unit tests; default is gcc-12
export CC=gcc-12
export GCOV=gcov-12

# build example app
git clean -dxf
make -f test/Makefile
# run libfuzzer detected crashes through for regression
(
    # shellcheck disable=SC2044
    for crashfile in $(find test/corpus/ -type f) ; do
        ./build/example/example < "$crashfile" || exit 1
    done
)

git clean -dxf
CFLAGS="-Weverything -Wno-error=reserved-id-macro -Wno-error=padded -Wno-error=declaration-after-statement -DNOCLI_RUNTIME_ECHO_CONTROL=0" CC=clang-14 NO_LCOV=1 make -f test/Makefile
git clean -dxf
make -f test/Makefile test --trace
git clean -dxf
# for coverage, disable asan (which inserts extra branches the test does not hit)
DISABLE_ASAN=1 make -f test/Makefile test
