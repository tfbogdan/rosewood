cmake_minimum_required(VERSION 3.10)


set(CLANG_LIBRARIES clangFrontend  clangSerialization clangDriver clangParse clangRewriteFrontend clangStaticAnalyzerFrontend clangSema)
set(CLANG_LIBRARIES ${CLANG_LIBRARIES} clangAnalysis clangEdit clangAST clangLex clangBasic clangTooling )

add_executable(mc 
	main.cpp
	clang_infrastructure.h
	MetaContext.h
	SourceFile.h
	SourceFile.cpp	
	SourceElements.h
	SourceElements.cpp
	clang_infrastructure.cpp
	GeneratingASTVisitor.h
	GeneratingASTVisitor.cpp
	CodeGeneratorBase.h
	CodeGeneratorBase.cpp
	RuntimeReflectionDataSourceGenerator.h 
	RuntimeReflectionDataSourceGenerator.cpp
	StaticReflectionDataHeaderGenerator.h
	StaticReflectionDataHeaderGenerator.cpp
	MetaPreambleHeaderGenerator.h
	MetaPreambleHeaderGenerator.cpp
	IdentifierRepository.h
	IdentifierRepository.cpp
)

target_link_libraries(mc
	PRIVATE
	LLVMX86AsmParser # MC, MCParser, Support, X86Desc, X86Info
	LLVMARMAsmParser
	LLVMX86Desc # MC, Support, X86AsmPrinter, X86Info
	LLVMX86AsmPrinter # MC, Support, X86Utils
	LLVMX86Info # MC, Support, Target
	LLVMARMInfo
	LLVMX86Utils # Core, Support
	LLVMipo
	LLVMScalarOpts
	LLVMInstCombine
	LLVMTransformUtils
	LLVMAnalysis
	LLVMTarget
	LLVMOption # Support
	LLVMMCParser # MC, Support
	LLVMMC # Object, Support
	LLVMObject # BitReader, Core, Support
	LLVMBitReader # Core, Support
	LLVMCore # Support
	LLVMSupport
)

target_link_libraries(mc PRIVATE ${CLANG_LIBRARIES} CONAN_PKG::fmt CONAN_PKG::jsonformoderncpp stdc++fs)
target_include_directories(mc PRIVATE ${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
target_compile_options(mc PRIVATE -fno-rtti)
target_compile_features(mc PRIVATE cxx_std_17)

install(
	TARGETS mc
	EXPORT mc
  RUNTIME
		DESTINATION bin
)
