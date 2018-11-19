
set(CLANG_LIBRARIES clangFrontend clangDriver clangParse clangSema)
set(CLANG_LIBRARIES ${CLANG_LIBRARIES} clangAST clangLex clangBasic clangTooling )

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
        # TDO: The following block should vary from platform to platform but this covers both linux and windows platforms so it's good enough right now
        LLVMX86Disassembler
        LLVMX86CodeGen
	LLVMX86AsmParser # MC, MCParser, Support, X86Desc, X86Info
	LLVMX86Desc # MC, Support, X86AsmPrinter, X86Info
	LLVMX86AsmPrinter # MC, Support, X86Utils
	LLVMX86Info # MC, Support, Target
	LLVMX86Utils # Core, Support

        LLVMOption # Support
	LLVMCore # Support
	LLVMSupport
)

target_link_libraries(rwc PRIVATE ${CLANG_LIBRARIES} CONAN_PKG::fmt CONAN_PKG::jsonformoderncpp)
if (NOT WIN32)
	target_link_libraries(rwc PRIVATE stdc++fs)
endif(NOT WIN32)

list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})
include (AddLLVM)
llvm_update_compile_flags(rwc)

target_include_directories(rwc PRIVATE ${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
target_compile_options(rwc PRIVATE -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
target_compile_features(rwc PRIVATE cxx_std_17)


install(
	TARGETS rwc
	EXPORT rosewood-exports
  RUNTIME
		DESTINATION bin
)

