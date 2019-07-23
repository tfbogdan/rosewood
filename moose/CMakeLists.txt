cmake_minimum_required(VERSION 3.9)

project(moose VERSION 0.0.1 LANGUAGES CXX)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

flex_target(scanner scanner.l ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)
bison_target(parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
add_flex_bison_dependency(scanner parser)

add_definitions(-DFMT_HEADER_ONLY) # global right now. should be added per target
                                   # use header only fmt as it's library is not compatible with older platforms

install(FILES cmake/rosewood.cmake DESTINATION lib/cmake/rosewood)

add_library(moose)

target_sources(moose PRIVATE
    ${FLEX_scanner_OUTPUTS}
    ${BISON_parser_OUTPUTS}
    # scanner.l
    # parser.y

    Lexer.cpp
    Lexer.h

    moose.cpp
    moose/moose.hpp

    ast.cpp
    moose/ast.h

    driver.h
    driver.cpp
)

target_include_directories(moose PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(moose PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(moose PUBLIC fmt::fmt rwruntime)
target_compile_features(moose PUBLIC cxx_std_17)
