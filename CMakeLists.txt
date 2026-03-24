project(FortressChronicle)

# Generates one .c source per header, each including a single header,
# and creates a target that compiles those generated C sources.
function(compile_headers_as_c out_target)
    set(options)
    set(oneValueArgs NAME)
    set(multiValueArgs HEADERS INCLUDE_DIRS COMPILE_DEFINITIONS COMPILE_OPTIONS)
    cmake_parse_arguments(CHC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT CHC_NAME)
        message(FATAL_ERROR "compile_headers_as_c: NAME is required")
    endif()

    if(NOT CHC_HEADERS)
        message(FATAL_ERROR "compile_headers_as_c: HEADERS is required")
    endif()

    set(generated_c_sources)
    foreach(header IN LISTS CHC_HEADERS)
        string(MAKE_C_IDENTIFIER "${header}" header_id)
        set(generated_c "${CMAKE_CURRENT_BINARY_DIR}/${CHC_NAME}_${header_id}.c")
        file(GENERATE
            OUTPUT "${generated_c}"
            CONTENT "/* Auto-generated. Compiles ${header} as C include test. */\n#include \"${header}\"\n")
        list(APPEND generated_c_sources "${generated_c}")
    endforeach()

    # Keep .c extension, but force C++ compilation so the same compiler mode is used as the plugin.
    set_source_files_properties(${generated_c_sources} PROPERTIES LANGUAGE CXX)

    add_library(${CHC_NAME} OBJECT ${generated_c_sources})
    target_include_directories(${CHC_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CHC_INCLUDE_DIRS})

    if(CHC_COMPILE_DEFINITIONS)
        target_compile_definitions(${CHC_NAME} PRIVATE ${CHC_COMPILE_DEFINITIONS})
    endif()

    if(CHC_COMPILE_OPTIONS)
        target_compile_options(${CHC_NAME} PRIVATE ${CHC_COMPILE_OPTIONS})
    endif()

    set(${out_target} ${CHC_NAME} PARENT_SCOPE)
endfunction()

add_subdirectory(sqliteWrapper) # sqlite wrapper library

# A list of source files
set(PROJECT_SRCS
    main.cpp
    DFDataLogger.cpp
    Deaths.cpp
)

# A list of headers
set(PROJECT_HDRS


)


# A list of headers to test for standalone capability (i.e. can be compiled from C files without C++ features).
set(TEST_HDRS
    Events.hpp
    Deaths.hpp
    Items.hpp
    Jobs.hpp
    Units.hpp
)

#define this to silence deprecation warnings from C++20 features used in the plugin
add_compile_definitions(_SILENCE_ALL_CXX20_DEPRECATION_WARNINGS)


list(APPEND PROJECT_SRCS ${PROJECT_HDRS})

# Optional target: build this target explicitly to verify headers can be compiled from C files.
compile_headers_as_c(FORTRESS_CHRONICLE_HEADER_C_TARGET
    NAME FortressChronicleHeaderC
    HEADERS ${TEST_HDRS}
)

# This CMake command registers the FortressChronicle plugin with DFHack.
# 
# Parameters:
# - FortressChronicle: The name of the plugin
# - ${PROJECT_SRCS}: Source files to compile for this plugin
# - LINK_LIBRARIES: Specifies libraries to link against
#   - DB: Database library for plugin data persistence
#   - sqlite3: SQLite3 library for database operations
#
# Note: PLUGIN_DEFAULT_ARGS are typically set by the parent dfhack_plugin() macro
# defined in the DFHack CMake configuration files (usually in cmake/ directory).
# They contain common default arguments like:
# - PLUGIN_VERSION
# - COMPILE_FLAGS
# - LINK_FLAGS
# - INSTALL_DESTINATION
# These are set via dfhack_plugin() macro definition and passed implicitly.
dfhack_plugin(FortressChronicle ${PROJECT_SRCS} LINK_LIBRARIES DB sqlite3)