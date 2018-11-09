cmake_minimum_required(VERSION 3.9.4)

function(target_public_headers target)
	target_sources(${target} PRIVATE ${ARGN})
	set_property(TARGET ${target} APPEND PROPERTY PUBLIC_HEADER  ${ARGN})
endfunction()

add_library(
	mcruntime 
	MetaClass.cpp
	MetaType.cpp
	MetaDecl.cpp
	MetadataStore.cpp
	Metal.cpp
)

target_public_headers(mcruntime 	
	${PROJECT_SOURCE_DIR}/include/mc/MetaClass.h
	${PROJECT_SOURCE_DIR}/include/mc/MetaType.h
	${PROJECT_SOURCE_DIR}/include/mc/Reflection.h
	${PROJECT_SOURCE_DIR}/include/mc/MetaDecl.h
	${PROJECT_SOURCE_DIR}/include/mc/MetadataStore.h
	${PROJECT_SOURCE_DIR}/include/mc/Metal.h
	${PROJECT_SOURCE_DIR}/include/mc/MetalConfig.h
	${PROJECT_SOURCE_DIR}/include/mc/QualType.h
)

target_include_directories(mcruntime 
	PUBLIC 
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>)

install(DIRECTORY ${CLANG_CMAKE_DIR}/../../clang/7.0.1/include DESTINATION lib/clang/7.0.1/include)

install(
	TARGETS mcruntime
	EXPORT mc
	ARCHIVE
		DESTINATION lib
	LIBRARY
		NAMELINK_ONLY
		DESTINATION lib
	PUBLIC_HEADER
		DESTINATION include/mc
)