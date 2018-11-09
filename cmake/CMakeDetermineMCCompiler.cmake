cmake_minimum_required(VERSION 3.10)


if (NOT CMAKE_MC_COMPILER)

if (TARGET mc)
  set(CMAKE_MC_COMPILER mc CACHE INTERNAL "Use mc from build directory")
else()
  find_program(mcBinary NAMES mc)
  if (not MC)
    message(FATAL_ERROR "Cannot find a metadata compiler.")
  endif()
  set(CMAKE_MC_COMPILER mc CACHE INTERNAL "Use mc from build directory")
endif()

endif()


configure_file(${CMAKE_CURRENT_LIST_DIR}/CMakeMCCompiler.cmake.in CMakeMCCompiler.cmake @ONLY)