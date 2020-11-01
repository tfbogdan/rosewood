cmake_minimum_required(VERSION 3.9)

function(metacompile_header target headerFile)
    get_target_property(TGT_INCLUDE_DIRS ${target} INCLUDE_DIRECTORIES)
    get_target_property(TGT_DEFS ${target} COMPILE_DEFINITIONS)
    set(includeDirs  "$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>")
    set(compileDefs  "$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>")
    set(cxxStandard "$<TARGET_PROPERTY:${target},CXX_STANDARD>")
    set(cxxStandardFlag "-std=c++${cxxStandard}")
    get_filename_component(filnenameWE ${headerFile} NAME_WE)

    set(outputCXXFile ${CMAKE_CURRENT_BINARY_DIR}/${filnenameWE}.metadata.h)
    set(outputJsonFile ${CMAKE_CURRENT_BINARY_DIR}/${filnenameWE}.metadata.json)

    add_custom_command(
        COMMAND rwc ${CMAKE_CURRENT_SOURCE_DIR}/${headerFile} -n ${filnenameWE} -o ${outputCXXFile} -j ${outputJsonFile} -- -x c++ "$<$<BOOL:${includeDirs}>:-I$<JOIN:${includeDirs},;-I>>" "$<$<BOOL:${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}>:-I$<JOIN:${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES},;-I>>" "$<$<BOOL:${compileDefs}>:-D$<JOIN:${compileDefs},;-D>>" ${cxxStandardFlag} -fsyntax-only -Wno-pragma-once-outside-header -nobuiltininc
        OUTPUT ${outputCXXFile} ${outputJsonFile}
        COMMENT "Generating reflection data for ${headerFile}"
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${headerFile} rwc
        IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/${headerFile}
        COMMAND_EXPAND_LISTS
        )
    target_sources(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${headerFile} ${outputCXXFile} ${outputJsonFile})
    target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
endfunction()


function(rosewood_reflect_on_headers targetName generatedTargetName headerFiles)
    set(includeDirs  "$<TARGET_PROPERTY:${targetName},INCLUDE_DIRECTORIES>")
    set(compileDefs  "$<TARGET_PROPERTY:${targetName},COMPILE_DEFINITIONS>")
    set(cxxStandard "$<TARGET_PROPERTY:${targetName},CXX_STANDARD>")
    set(cxxStandardFlag "-std=c++${cxxStandard}")

    if (NOT TARGET ${generatedTargetName})
        add_library(${generatedTargetName} OBJECT)
        # It might be better to allow users to define the visibility of this include path
        target_include_directories(${targetName} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
        target_sources(${targetName} PRIVATE $<TARGET_OBJECTS:${generatedTargetName}>)
        target_include_directories(${generatedTargetName} PRIVATE ${includeDirs})
        target_compile_definitions(${generatedTargetName} PRIVATE ${compileDefs})
        target_link_libraries(${generatedTargetName} PRIVATE rwruntime)
        target_compile_features(${generatedTargetName} PRIVATE cxx_std_${cxxStandard})
    else()
        get_target_property(genTargetDir ${generatedTargetName} SOURCE_DIR)
        if (NOT genTargetDir STREQUAL CMAKE_CURRENT_SOURCE_DIR)
            message(FATAL "Generated targets may not be user in more than 1 directory. Try another name that hasn't already been used in another directory.")
        endif()
    endif()

    foreach(headerFile ${headerFiles})
        get_filename_component(filnenameWE ${headerFile} NAME_WE)

        set(outputCXXFile ${CMAKE_CURRENT_BINARY_DIR}/${filnenameWE}.metadata.h)
        set(outputJsonFile ${CMAKE_CURRENT_BINARY_DIR}/${filnenameWE}.metadata.json)

        add_custom_command(
            COMMAND rwc ${CMAKE_CURRENT_SOURCE_DIR}/${headerFile} -n ${filnenameWE} -o ${outputCXXFile} -j ${outputJsonFile} -- -x c++ "$<$<BOOL:${includeDirs}>:-I$<JOIN:${includeDirs},;-I>>" "$<$<BOOL:${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}>:-I$<JOIN:${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES},;-I>>" "$<$<BOOL:${compileDefs}>:-D$<JOIN:${compileDefs},;-D>>" ${cxxStandardFlag} -fsyntax-only -Wno-pragma-once-outside-header -nobuiltininc
            OUTPUT ${outputCXXFile} ${outputJsonFile}
            COMMENT "Generating reflection data for ${headerFile}"
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${headerFile} rwc
            IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/${headerFile}
            COMMAND_EXPAND_LISTS
            )

        set (forcedDependencyFile ${CMAKE_CURRENT_BINARY_DIR}/__${filnenameWE}_forceinclude.cpp)
        file(GENERATE OUTPUT ${forcedDependencyFile} CONTENT "#include \"${outputCXXFile}\"")
        target_sources(${generatedTargetName} PRIVATE ${outputCXXFile} ${outputJsonFile} ${forcedDependencyFile})
    endforeach()
endfunction()
