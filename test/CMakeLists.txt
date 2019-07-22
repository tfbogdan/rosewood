include(CTest)

if (BUILD_TESTING)
    find_package(GTest REQUIRED)

    add_executable(rwtest BasicDefinitions.cpp main.cpp)
    metacompile_header(rwtest BasicDefinitions.h)
    metacompile_header(rwtest TemplateDeclarations.h)
    target_link_libraries(rwtest PRIVATE rwruntime GTest::GTest)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options(rwtest PRIVATE -Wall -pedantic -g -O0 --coverage)
        target_link_libraries(rwtest PRIVATE --coverage)
    endif () 
    
endif(BUILD_TESTING)
add_test(NAME rwtest COMMAND rwtest)
