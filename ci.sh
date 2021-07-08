#!/usr/bin/env bash

# Simple test script to run the tests in docker

# Error on any non-zero command, and print the commands as they're run
set -ex
set -o pipefail

# Make sure we have the docker utility
if ! command -v docker; then
    echo "🐋 Please install docker first 🐋"
    exit 1
fi

# Set the docker image name to default to repo basename
DOCKER_IMAGE_NAME=${DOCKER_IMAGE_NAME:-$(basename -s .git "$(git remote --verbose | awk 'NR==1 { print tolower($2) }')")}

# build the docker image
DOCKER_BUILDKIT=1 docker build -t "$DOCKER_IMAGE_NAME" --build-arg "UID=$(id -u)" -f Dockerfile .

# execute tox in the docker container. don't run in parallel; conda has issues
# when we do this (pkg cache operations are not atomic!)
docker run --rm -v "$(pwd)":/mnt/workspace -t "$DOCKER_IMAGE_NAME" bash -c '
    set -ex
    set -o pipefail

    # commit checker
    #pre-commit run --all-files

    # compilation + unit tests
    git clean -dxf
    make -C test all
    make -C test clean
    make -C test test
    git clean -dxf
    make -C test -f Makefile_cortexm4.mk
    git clean -dxf
    CFLAGS="-DNOCLI_CONFIG_HELP_COMMAND=0" make -C test -f Makefile_cortexm4.mk
    git clean -dxf
    CFLAGS="-Weverything -Wno-error=reserved-id-macro -Wno-error=padded" CC=clang-12 GCOV="llvm-cov-12 gcov" make -C test test
    git clean -dxf
'
