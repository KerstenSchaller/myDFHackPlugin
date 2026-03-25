project(FortressChronicle)

#set(CMAKE_CXX_STANDARD 17)


add_subdirectory(sqliteWrapper) # sqlite wrapper library


add_subdirectory(DataLogging) # included as sources and header files, as well as a interface lib for include folders

# A list of source files
set(PROJECT_SRCS
    main.cpp
    ${DFDATALOGGER_SOURCES}
)

# A list of headers
set(PROJECT_HDRS
    ${DFDATALOGGER_HEADERS}
)




#define this to silence deprecation warnings from C++20 features used in the plugin
add_compile_definitions(_SILENCE_ALL_CXX20_DEPRECATION_WARNINGS)


list(APPEND PROJECT_SRCS ${PROJECT_HDRS})


dfhack_plugin(FortressChronicle ${PROJECT_SRCS} 
    LINK_LIBRARIES DB sqlite3 DFDataLoggerIncludes
)