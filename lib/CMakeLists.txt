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


install(TARGETS rwruntime EXPORT rosewood-exports
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/rosewood
    DESTINATION ${INCLUDE_INSTALL_DIR}
)
