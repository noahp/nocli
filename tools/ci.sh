#!/usr/bin/env bash

set -ex
set -o pipefail

# get the parent directory of this file
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT_DIR" || exit 1

# commit checker
uvx prek run --all-files

# build for cortex-m4
git clean -dxf
make -f test/Makefile_cortexm4.mk
git clean -dxf
CFLAGS="-DNOCLI_CONFIG_HELP_COMMAND=0" make -f test/Makefile_cortexm4.mk
git clean -dxf
CFLAGS="-DNOCLI_CONFIG_HELP_COMMAND=0 -DNOCLI_QUOTED_ARGS_SUPPORT=0" make -f test/Makefile_cortexm4.mk

# compilation + unit tests
export CC=gcc
export GCOV=gcov

# build example app
git clean -dxf
make -f test/Makefile
# run libfuzzer detected crashes through for regression
(
	# shellcheck disable=SC2044
	for crashfile in $(find test/corpus/ -type f); do
		./build/example/example <"$crashfile" || exit 1
	done
)

git clean -dxf
CLANG_CFLAGS="\
  -Weverything\
 -Wno-error=reserved-id-macro\
 -Wno-error=padded\
 -Wno-declaration-after-statement\
 -Wno-unsafe-buffer-usage\
 -DNOCLI_RUNTIME_ECHO_CONTROL=0"
CFLAGS="${CLANG_CFLAGS}" CC=clang-20 NO_LCOV=1 make -f test/Makefile
git clean -dxf
make -f test/Makefile test --trace
git clean -dxf
# for coverage, disable asan (which inserts extra branches the test does not hit)
DISABLE_ASAN=1 make -f test/Makefile test
