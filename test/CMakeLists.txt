include(CTest)

if (BUILD_TESTING)
    add_executable(rwtest BasicDefinitions.cpp main.cpp)
    metacompile_header(rwtest BasicDefinitions.h)
    metacompile_header(rwtest TemplateDeclarations.h)
    target_link_libraries(rwtest PRIVATE rwruntime CONAN_PKG::gtest)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options(rwtest PRIVATE -Wall -pedantic -g -O0 --coverage)
        target_link_libraries(rwtest PRIVATE --coverage)
    endif () 
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		# turns out our object file is too big for msvc so we have to use /bigobj to work around this arbitrary limitation
		message(STATUS "Using MSVC, enabling /bigobj")
                target_compile_options(rwtest PRIVATE /bigobj)
	endif()
    
endif(BUILD_TESTING)
add_test(NAME rwtest COMMAND rwtest)
