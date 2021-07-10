FROM ubuntu:hirsute

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get -y install \
    build-essential \
    cppcheck \
    clang-12 \
    clang-format \
    clang-tidy-12 \
    gcc-11 \
    git \
    lcov \
    lld-12 \
    llvm-12 \
    pv \
    python3-pip \
    wget

ARG ARM_URL=https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2?revision=ca0cbf9c-9de2-491c-ac48-898b5bbc0443&la=en&hash=68760A8AE66026BCF99F05AC017A6A50C6FD832A
RUN wget -nv --show-progress --progress=bar:force:noscroll ${ARM_URL} -O /opt/gcc-arm-none-eabi.tar.bz2 && \
    mkdir -p /opt/gcc-arm-none-eabi && \
    pv --force /opt/gcc-arm-none-eabi.tar.bz2 | tar xj --directory /opt/gcc-arm-none-eabi --strip-components 1
ENV PATH=/opt/gcc-arm-none-eabi/bin:${PATH}

# installing non-default clang-tidy means we need to symlink
RUN ln -s clang-tidy-12 /usr/bin/clang-tidy

# get user id from build arg, so we can have read/write access to directories
# mounted inside the container. only the UID is necessary, UNAME just for
# cosmetics
ARG UID=1010
ARG UNAME=builder

RUN useradd --uid $UID --create-home --user-group ${UNAME} && \
    echo "${UNAME}:${UNAME}" | chpasswd && adduser ${UNAME} sudo

USER ${UNAME}

ENV LANG=C.UTF-8 LC_ALL=C.UTF-8

ENV PATH /home/${UNAME}/.local/bin:$PATH

RUN pip3 install pre-commit==2.13.0 compiledb==0.10.1

WORKDIR /mnt/workspace
