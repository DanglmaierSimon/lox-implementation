FROM ubuntu:21.10

RUN apt-get update && DEBIAN_FRONTEND="noninteractive" TZ="Europe/Vienna" apt-get install -y \
    git \
    cmake \
    clang-13 \
    ninja-build

COPY CMakeLists.txt /usr/src/cpplox/CMakeLists.txt
COPY benchmark /usr/src/cpplox/benchmark
COPY cpplox /usr/src/cpplox/cpplox
COPY include /usr/src/cpplox/include
COPY source /usr/src/cpplox/source
COPY tests /usr/src/cpplox/tests

RUN mkdir -p /usr/src/cpplox/build

WORKDIR /usr/src/cpplox/build

ARG CXX=clang++-13
ARG BUILD_TYPE=Debug
ARG SANITIZER

RUN echo "Running cmake..." && cmake .. -G "Ninja"  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DSANITIZER=${SANITIZER}

RUN ninja all

CMD [ "ctest", ".", "--output-on-failure", "-j 8", "--shuffle-random" , "--timeout", "5"]