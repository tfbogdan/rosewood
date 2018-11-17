include(CTest)

if (BUILD_TESTING)
    add_executable(mc_test BasicDefinitions.cpp main.cpp)
    metacompile_header(mc_test BasicDefinitions.h)
    metacompile_header(mc_test TemplateDeclarations.h)
    target_link_libraries(mc_test PRIVATE mcruntime CONAN_PKG::gtest --coverage)
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        target_compile_options(mc_test PRIVATE -Wall -pedantic -g -O0 --coverage)
    endif () 
    
endif(BUILD_TESTING)
add_test(NAME mc_tests COMMAND mc_test)
