cmake_minimum_required(VERSION 3.9)

add_library(rwruntime
    STATIC
    runtime.cpp
    ${PROJECT_SOURCE_DIR}/include/rosewood/rosewood.hpp
    ${PROJECT_SOURCE_DIR}/include/rosewood/runtime.hpp
    ${PROJECT_SOURCE_DIR}/include/rosewood/index.hpp
    ${PROJECT_SOURCE_DIR}/include/rosewood/type.hpp
    index.cpp
)

target_include_directories(rwruntime
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_compile_features(rwruntime PUBLIC cxx_std_17)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
install(DIRECTORY ${CLANG_CMAKE_DIR}/../../clang/${LLVM_PACKAGE_VERSION}/include DESTINATION lib/clang/${LLVM_PACKAGE_VERSION}/include)
file(COPY ${CLANG_CMAKE_DIR}/../../clang/${LLVM_PACKAGE_VERSION}/include DESTINATION clang/${LLVM_PACKAGE_VERSION})
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

install(TARGETS rwruntime EXPORT rosewood-exports
    ARCHIVE DESTINATION lib)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/rosewood
    DESTINATION include
)
