cmake_minimum_required(VERSION 3.9)

add_library(mcruntime STATIC dynamic_mc.cpp ${PROJECT_SOURCE_DIR}/include/mc/mc.hpp ${PROJECT_SOURCE_DIR}/include/mc/dynamic_mc.hpp)

target_include_directories(mcruntime 
        PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>)

target_compile_features(mcruntime PUBLIC cxx_std_17)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
install(DIRECTORY ${CLANG_CMAKE_DIR}/../../clang/${LLVM_PACKAGE_VERSION}/include DESTINATION lib/clang/${LLVM_PACKAGE_VERSION}/include)
file(COPY ${CLANG_CMAKE_DIR}/../../clang/${LLVM_PACKAGE_VERSION}/include DESTINATION clang/${LLVM_PACKAGE_VERSION})
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include
    DESTINATION include/mc
)
