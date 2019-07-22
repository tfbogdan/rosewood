cmake_minimum_required(VERSION 3.9)

project(rosewood VERSION 0.0.2 LANGUAGES CXX)
find_package(Clang REQUIRED CONFIG)
find_package(fmt REQUIRED)
# find_package(nlohmann_json REQUIRED)
enable_testing()

include(cmake/rosewood.cmake)
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(moose)



configure_file(cmake/rosewood.cmake lib/cmake/rosewood/rosewood.cmake COPYONLY)
install(FILES cmake/rosewood.cmake DESTINATION lib/cmake/rosewood)

configure_file(cmake/rosewood-config.cmake.in lib/cmake/rosewood/rosewood-config.cmake @ONLY)

export(TARGETS rwc rwruntime FILE lib/cmake/rosewood/mc-exports.cmake)
install(EXPORT rosewood-exports DESTINATION lib/cmake/rosewood)
install(FILES ${PROJECT_BINARY_DIR}/lib/cmake/rosewood/rosewood-config.cmake DESTINATION lib/cmake/rosewood)
