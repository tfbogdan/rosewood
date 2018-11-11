cmake_minimum_required(VERSION 3.9.4)

add_library(mcruntime INTERFACE)

target_include_directories(mcruntime 
        INTERFACE
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>)

target_compile_features(mcruntime INTERFACE cxx_std_17)

install(DIRECTORY ${CLANG_CMAKE_DIR}/../../clang/${LLVM_PACKAGE_VERSION}/include DESTINATION lib/clang/${LLVM_PACKAGE_VERSION}/include)
file(COPY ${CLANG_CMAKE_DIR}/../../clang/${LLVM_PACKAGE_VERSION}/include DESTINATION clang/${LLVM_PACKAGE_VERSION})

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include
    DESTINATION include/mc
)
