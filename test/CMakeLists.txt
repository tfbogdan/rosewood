include(CTest)

if (BUILD_TESTING)
    add_executable(mc_test Jinx.cpp main.cpp)
    metacompile_header(mc_test Jinx.h)
    target_link_libraries(mc_test PRIVATE mcruntime CONAN_PKG::gtest)
endif(BUILD_TESTING)
