FROM debian:10.1
LABEL maintainer "k-okubo <koichiro.okubo@gmail.com>"

RUN apt-get update && apt-get install -y \
        cmake \
        curl \
        gcc \
        g++ \
        python \
        xz-utils \
        && apt-get clean \
        && rm -rf /var/lib/apt/lists/*

WORKDIR /tsugu

COPY external ./external
RUN cd external \
        && make -j$(nproc)

COPY . ./
RUN mkdir build \
        && cd build \
        && cmake .. \
        && make -j$(nproc)
