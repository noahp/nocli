# This image is pushed to ghcr.io/noahp/nocli

FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get -y install --no-install-recommends \
    build-essential \
    cppcheck \
    clang-20 \
    clang-format \
    clang-tidy-20 \
    curl \
    gcc-14 \
    git \
    lcov \
    libclang-rt-20-dev \
    lld-20 \
    llvm-20 \
    python3-pip \
    shfmt \
    wget \
    xz-utils \
    # gcc-arm-none-eabi and libnewlib-arm-none-eabi are needed for embedded build
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi

# installing non-default clang-tidy means we need to symlink
RUN ln -s clang-tidy-20 /usr/bin/clang-tidy

ENV LANG=C.UTF-8 LC_ALL=C.UTF-8

# install uv
RUN curl -LsSf https://astral.sh/uv/0.9.14/install.sh | sh

ENV PATH="/root/.local/bin:${PATH}"
