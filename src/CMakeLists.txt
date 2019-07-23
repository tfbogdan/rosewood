
add_executable(rwc
    main.cpp
    ClangInfrastructure.h
    ClangInfrastructure.cpp
    ReflectionDataGenerator.h
    ReflectionDataGenerator.cpp
    IdentifierHelper.h
    IdentifierHelper.cpp
    IdentifierRepository.h
    IdentifierRepository.cpp
)

target_link_libraries(rwc
    PRIVATE
    clangTooling
    clangAST
    clangBasic
    clangFrontend
    clangSerialization
    clangSema

    fmt::fmt
    nlohmann_json::nlohmann_json
)

if (NOT WIN32)
	target_link_libraries(rwc PRIVATE stdc++fs)
endif(NOT WIN32)

#list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})
#include (AddLLVM)
#llvm_update_compile_flags(rwc)
llvm_config(rwc USE_SHARED Option Core Support)

target_include_directories(rwc PRIVATE ${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
target_compile_options(rwc PRIVATE -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
target_compile_features(rwc PRIVATE cxx_std_17)


install(
    TARGETS rwc
    EXPORT rosewood-exports
    RUNTIME
        DESTINATION bin
)

