cmake_minimum_required(VERSION 3.10)

function(metacompile_header target headerFile)
	get_target_property(TGT_INCLUDE_DIRS ${target} INCLUDE_DIRECTORIES)
	get_target_property(TGT_DEFS ${target} COMPILE_DEFINITIONS)
	set(includeDirs "$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>")
	set(compileDefs "$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>")
	get_filename_component(filnenameWE ${headerFile} NAME_WE)

        set(outputCXXFile ${CMAKE_CURRENT_BINARY_DIR}/${filnenameWE}.metadata.h)
	set(outputJsonFile ${CMAKE_CURRENT_BINARY_DIR}/${filnenameWE}.metadata.json)

	add_custom_command(
                COMMAND mc ${CMAKE_CURRENT_SOURCE_DIR}/${headerFile} -n ${filnenameWE} -o ${outputCXXFile} -j ${outputJsonFile} -- "$<$<BOOL:${includeDirs}>:-I$<JOIN:${includeDirs},;-I>>" "$<$<BOOL:${compileDefs}>:-D$<JOIN:${compileDefs},;-D>>" -fsyntax-only -Wno-pragma-once-outside-header
                OUTPUT ${outputCXXFile} ${outputJsonFile}
		COMMENT "Generating reflection data for ${headerFile}"
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${headerFile} mc
		IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/${headerFile}
		COMMAND_EXPAND_LISTS
	)
        target_sources(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${headerFile} ${outputCXXFile} ${outputJsonFile})
        target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
	set_property(TARGET ${target} APPEND PROPERTY MC_METADATA_FILES ${outputJsonFile})
endfunction()


function(assemble_reflection_data target)
	get_property(MetadataFiles TARGET ${target} PROPERTY MC_METADATA_FILES)
	get_property(TargetOutputName TARGET ${target} PROPERTY NAME)

	message(STATUS "Collecting reflection data from ${MetadataFiles}")

	set(reflectionFiles "$<TARGET_PROPERTY:${target},MC_METADATA_FILES>")
	set(outputCXXFile ${CMAKE_CURRENT_BINARY_DIR}/${TargetOutputName}.metadata.cpp)
	set(outputJsonFile ${CMAKE_CURRENT_BINARY_DIR}/${TargetOutputName}.metadata.json)

	add_custom_command(
                COMMAND mc "$<$<BOOL:${reflectionFiles}>: $<JOIN:${reflectionFiles},; >>" -n none -o ${outputCXXFile} -j ${outputJsonFile} -- "$<$<BOOL:${includeDirs}>:-I$<JOIN:${includeDirs},;-I>>" "$<$<BOOL:${compileDefs}>:-D$<JOIN:${compileDefs},;-D>>" -fsyntax-only -Wno-pragma-once-outside-header
                OUTPUT ${outputCXXFile} ${outputJsonFile}
		COMMENT "Assembling reflection data for ${TargetOutputName}"
		DEPENDS ${MetadataFiles} mc
		COMMAND_EXPAND_LISTS
	)

	target_sources(${target} PRIVATE ${outputCXXFile} ${outputJsonFile})

	# TDO, obviously, a lot of things

endfunction()
