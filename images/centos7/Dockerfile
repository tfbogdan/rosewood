FROM centos:7

RUN yum install git -y
RUN yum install epel-release -y
RUN yum install centos-release-scl -y
RUN yum install devtoolset-8 -y

RUN yum install wget -y
RUN wget https://github.com/Kitware/CMake/releases/download/v3.15.0/cmake-3.15.0-Linux-x86_64.sh
RUN chmod +x cmake-3.15.0-Linux-x86_64.sh
RUN /cmake-3.15.0-Linux-x86_64.sh --prefix=/usr --exclude-subdir --skip-license

RUN git clone https://github.com/fmtlib/fmt.git
WORKDIR fmt/build
RUN scl enable devtoolset-8 "cmake /fmt -DCMAKE_INSTALL_PREFIX=/usr"
RUN cmake --build /fmt/build --target install

RUN yum install -y ninja-build

# get llvm and clang and compile them
WORKDIR /
RUN wget http://releases.llvm.org/8.0.0/llvm-8.0.0.src.tar.xz
RUN wget http://releases.llvm.org/8.0.0/cfe-8.0.0.src.tar.xz
WORKDIR /llvm
RUN tar xvf /llvm-8.0.0.src.tar.xz --strip-components=1 --no-same-owner
WORKDIR /llvm/tools/clang
RUN tar xvf /cfe-8.0.0.src.tar.xz --strip-components=1 --no-same-owner

WORKDIR /llvm/build
RUN scl enable devtoolset-8 "cmake /llvm -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=Off -DLLVM_TARGETS_TO_BUILD=\"X86\" -GNinja -DCMAKE_BUILD_TYPE=Release"
RUN cmake --build /llvm/build --target install
