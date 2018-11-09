cmake_minimum_required(VERSION 3.10)

project(mc VERSION 0.0.0 LANGUAGES CXX)
find_package(Clang REQUIRED)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.12/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(CONANFILE conanfile.txt
                BASIC_SETUP CMAKE_TARGETS
                BUILD missing)


add_subdirectory(lib)
add_subdirectory(src)


install(EXPORT mc DESTINATION lib/cmake/mc NAMESPACE mc)
configure_file(cmake/mc-config.cmake.in lib/cmake/mc/mc-config.cmake @ONLY)
export(TARGETS mc mcruntime FILE lib/cmake/mc/mc-exports.cmake)
