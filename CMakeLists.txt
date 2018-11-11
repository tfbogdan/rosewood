cmake_minimum_required(VERSION 3.9)

project(mc VERSION 0.0.1 LANGUAGES CXX)
find_package(Clang REQUIRED CONFIG)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.12/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(CONANFILE conanfile.txt
                BASIC_SETUP CMAKE_TARGETS
                BUILD missing)

include(cmake/mc.cmake)
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(test)


install(EXPORT mc DESTINATION lib/cmake/mc NAMESPACE mc)

configure_file(cmake/mc.cmake lib/cmake/mc/mc.cmake COPYONLY)
install(FILES cmake/mc.cmake DESTINATION lib/cmake/mc/mc.cmake)

configure_file(cmake/mc-config.cmake.in lib/cmake/mc/mc-config.cmake @ONLY)
configure_file(cmake/CMakeDetermineMcCompiler.cmake.in lib/cmake/mc/CMakeDetermineMcCompiler.cmake @ONLY)
configure_file(cmake/CMakeMcCompiler.cmake.in lib/cmake/mc/CMakeMcCompiler.cmake.in COPYONLY)
configure_file(cmake/CMakeMcInformation.cmake.in lib/cmake/mc/CMakeMcInformation.cmake @ONLY)
configure_file(cmake/CMakeTestMcCompiler.cmake.in lib/cmake/mc/CMakeTestMcCompiler.cmake @ONLY)

export(TARGETS mc mcruntime FILE lib/cmake/mc/mc-exports.cmake)
