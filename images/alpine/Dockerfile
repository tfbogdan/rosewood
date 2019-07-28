FROM alpine

RUN apk add cmake clang-dev g++ gtest-dev make git llvm-static clang-static llvm-dev py3-pip
RUN pip3 install gcovr

RUN git clone https://github.com/fmtlib/fmt.git
WORKDIR /fmtbuild
RUN cmake /fmt -DFMT_TEST=Off -DFMT_DOC=Off -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
RUN cmake --build . --target install

WORKDIR /