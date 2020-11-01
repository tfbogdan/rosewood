cmake_minimum_required(VERSION 3.9)

project(
    rosewood
    VERSION 0.1.0
    DESCRIPTION "C++ Reflection framework"
    HOMEPAGE_URL https://gitlab.com/tfbogdan/rosewood/
    LANGUAGES CXX
)

find_package(Clang CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
#find_package(nlohmann_json REQUIRED)

# install path defaults =
set(LIB_INSTALL_DIR "lib" CACHE PATH "lib directory under cmake_install_prefix")
set(INCLUDE_INSTALL_DIR "include" CACHE PATH "include directory under cmake_install_prefix")
set(BIN_INSTALL_DIR "bin" CACHE PATH "binary directory under cmake_install_prefix")
set(CMAKE_INSTALL_DIR "${LIB_INSTALL_DIR}/cmake" CACHE PATH "path for cmake files")

enable_testing()

include(cmake/rosewood.cmake)
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(test)

if (WITH_MOOSE)
    add_subdirectory(moose)
endif()


configure_file(cmake/rosewood.cmake lib/cmake/rosewood/rosewood.cmake COPYONLY)
install(FILES cmake/rosewood.cmake DESTINATION ${CMAKE_INSTALL_DIR}/rosewood)

configure_file(cmake/rosewood-config.cmake.in lib/cmake/rosewood/rosewood-config.cmake @ONLY)

export(TARGETS rwc rwruntime FILE lib/cmake/rosewood/rosewood-exports.cmake)
install(EXPORT rosewood-exports DESTINATION ${CMAKE_INSTALL_DIR}/rosewood)
install(FILES ${PROJECT_BINARY_DIR}/lib/cmake/rosewood/rosewood-config.cmake DESTINATION ${CMAKE_INSTALL_DIR}/rosewood)
