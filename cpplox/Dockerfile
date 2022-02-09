FROM ubuntu:21.10

RUN apt-get update && DEBIAN_FRONTEND="noninteractive" TZ="Europe/Vienna" apt-get install -y \
    git \
    cmake \
    clang-13 \
    ninja-build

COPY . /usr/src/cpplox

RUN mkdir -p /usr/src/cpplox/build

WORKDIR /usr/src/cpplox/build

ARG CXX=clang++-13
ARG BUILD_TYPE=Debug

RUN echo "Running cmake..." && cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -G "Ninja"

RUN ninja all

CMD [ "ctest", ".", "--output-on-failure", "-j 8", "--shuffle-random" ]