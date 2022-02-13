FROM ubuntu:21.10

RUN apt-get update && DEBIAN_FRONTEND="noninteractive" TZ="Europe/Vienna" apt-get install -y \
    git \
    cmake \
    clang-13 \
    ninja-build \
    curl

ENV CXX=/usr/bin/clang++-13
ENV CC=/usr/bin/clang-13

RUN mkdir -p /usr/src/

# install fmt lib 
WORKDIR /usr/src
RUN curl -SL https://github.com/fmtlib/fmt/archive/refs/tags/8.1.1.tar.gz --output /usr/src/fmt-src.tar.gz \
    && tar -xf fmt-src.tar.gz \
    && rm -f /usr/src/fmt-src.tar.gz

RUN mkdir -p  /usr/src/fmt-8.1.1/build

WORKDIR /usr/src/fmt-8.1.1/build

RUN cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DFMT_INSTALL=ON -DFMT_TEST=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON

RUN ninja install && rm -rf /usr/src/fmt-8.1.1/build

# install googletest lib
WORKDIR /usr/src
RUN curl -SL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz --output /usr/src/gtest-src.tar.gz \
    && tar -xf gtest-src.tar.gz \
    && rm -f /usr/src/gtest-src.tar.gz

RUN mkdir -p  /usr/src/googletest-release-1.11.0/build

WORKDIR /usr/src/googletest-release-1.11.0/build

RUN cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release -Dgtest_build_tests=OFF -Dgtest_build_samples=OFF -DINSTALL_GTEST=ON -DBUILD_GMOCK=ON

RUN ninja install && rm -rf /usr/src/googletest-release-1.11.0/build

WORKDIR /usr/src

COPY CMakeLists.txt /usr/src/cpplox/CMakeLists.txt
COPY benchmark /usr/src/cpplox/benchmark
COPY cpplox /usr/src/cpplox/cpplox
COPY include /usr/src/cpplox/include
COPY source /usr/src/cpplox/source

RUN mkdir -p /usr/src/cpplox/build

WORKDIR /usr/src/cpplox/build

ARG BUILD_TYPE=Debug
ARG SANITIZER

RUN echo "Building library..." && cmake .. -G "Ninja"  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DSANITIZER=${SANITIZER} -DBUILD_LOX_TESTS=OFF

RUN ninja cpplox lox

COPY tests /usr/src/cpplox/tests

RUN echo "Building tests..." && cmake .. -G "Ninja"  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DSANITIZER=${SANITIZER} -DBUILD_LOX_TESTS=ON

RUN ninja all

ENV LD_LIBRARY_PATH=/usr/local/lib/

CMD [ "ctest", ".", "--output-on-failure", "-j 8", "--shuffle-random" , "--timeout", "5"]