FROM ubuntu:23.10

RUN apt-get update && DEBIAN_FRONTEND="noninteractive" TZ="Europe/Vienna" apt-get install -y \
    git \
    cmake \
    clang-13 \
    ninja-build \
    curl

ENV CXX=/usr/bin/clang++-13
ENV CC=/usr/bin/clang-13

RUN mkdir -p /usr/src/
WORKDIR /usr/src

COPY CMakeLists.txt /usr/src/cpplox/CMakeLists.txt
COPY src /usr/src/cpplox/src

RUN mkdir -p /usr/src/cpplox/build

WORKDIR /usr/src/cpplox/build

ARG BUILD_TYPE=Debug

RUN echo "Building library..." && cmake .. -G "Ninja"  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DBUILD_LOX_TESTS=OFF

RUN ninja cpplox lox

COPY tests /usr/src/cpplox/tests

RUN echo "Building tests..." && cmake .. -G "Ninja"  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DBUILD_LOX_TESTS=ON

RUN ninja all

ENV LD_LIBRARY_PATH=/usr/local/lib/

CMD [ "ctest", ".", "--output-on-failure", "-j 8", "--shuffle-random" , "--timeout", "5"]