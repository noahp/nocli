# This image is pushed to ghcr.io/noahp/nocli

FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get -y install \
    build-essential \
    cppcheck \
    clang-14 \
    clang-format \
    clang-tidy-14 \
    gcc-12 \
    git \
    lcov \
    lld-14 \
    llvm-14 \
    python3-pip \
    wget \
    xz-utils

# Use bash for following commands
SHELL ["/bin/bash", "-c"]

ARG ARM_GCC_URL=https://developer.arm.com/-/media/Files/downloads/gnu/12.2.rel1/binrel/arm-gnu-toolchain-12.2.rel1-x86_64-arm-none-eabi.tar.xz
ARG ARM_GCC_SHA256=4be93d0f9e96a15addd490b6e237f588c641c8afdf90e7610a628007fc96867

RUN wget -q --show-progress --progress=bar:force:noscroll ${ARM_GCC_URL} -O - | \
    tee >(tar -x --xz) | sha256sum | grep -q "${ARM_GCC_SHA256}  -" \
    && mv *arm-none-eabi* /opt/gcc-arm-none-eabi

ENV PATH=/opt/gcc-arm-none-eabi/bin:${PATH}

# installing non-default clang-tidy means we need to symlink
RUN ln -s clang-tidy-14 /usr/bin/clang-tidy

ENV LANG=C.UTF-8 LC_ALL=C.UTF-8

RUN pip3 install pre-commit==2.21.0 compiledb==0.10.1
