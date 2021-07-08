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
    pre-commit run --all-files --verbose

    # compilation + unit tests
    git clean -dxf
    make -f test/Makefile
    make -f test/Makefile clean
    make -f test/Makefile test
    git clean -dxf
    make -f test/Makefile_cortexm4.mk
    git clean -dxf
    CFLAGS="-DNOCLI_CONFIG_HELP_COMMAND=0" make -f test/Makefile_cortexm4.mk
    git clean -dxf
    CFLAGS="-Weverything -Wno-error=reserved-id-macro -Wno-error=padded" CC=clang-12 NO_LCOV=1 make -f test/Makefile
    git clean -dxf
'
