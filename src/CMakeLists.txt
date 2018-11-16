
set(CLANG_LIBRARIES clangFrontend clangDriver clangParse clangSema)
set(CLANG_LIBRARIES ${CLANG_LIBRARIES} clangAST clangLex clangBasic clangTooling )

add_executable(mc 
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

target_link_libraries(mc
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

target_link_libraries(mc PRIVATE ${CLANG_LIBRARIES} CONAN_PKG::fmt CONAN_PKG::jsonformoderncpp)
if (NOT WIN32) 
	target_link_libraries(mc PRIVATE stdc++fs)
endif(NOT WIN32)

target_include_directories(mc PRIVATE ${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
target_compile_options(mc PRIVATE -fno-rtti)
target_compile_features(mc PRIVATE cxx_std_17)

install(
	TARGETS mc
	EXPORT mc
  RUNTIME
		DESTINATION bin
)

