cmake_minimum_required(VERSION 3.9)

project(rosewood VERSION 0.0.2 LANGUAGES CXX)
find_package(Clang REQUIRED CONFIG)
enable_testing()
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.12/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
set (BUILD_CONAN_DEPENDENCIES missing CACHE STRING "When calling conan, this will be used for the build parameter. ")
conan_cmake_run(CONANFILE conanfile.txt
                BASIC_SETUP CMAKE_TARGETS
                BUILD ${BUILD_CONAN_DEPENDENCIES})

add_definitions(-DFMT_HEADER_ONLY) # global right now. should be added per target
                                   # use header only fmt as it's library is not compatible with older platforms
include(cmake/rosewood.cmake)
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(test)



configure_file(cmake/rosewood.cmake lib/cmake/rosewood/rosewood.cmake COPYONLY)
install(FILES cmake/rosewood.cmake DESTINATION lib/cmake/rosewood)

configure_file(cmake/rosewood-config.cmake.in lib/cmake/rosewood/rosewood-config.cmake @ONLY)

export(TARGETS rwc rwruntime FILE lib/cmake/rosewood/mc-exports.cmake)
install(EXPORT rosewood-exports DESTINATION lib/cmake/rosewood)
install(FILES ${PROJECT_BINARY_DIR}/lib/cmake/rosewood/rosewood-config.cmake DESTINATION lib/cmake/rosewood)
