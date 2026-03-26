
# Manifest-only: this directory does not create a build target.
# The parent target can consume these lists with target_sources(...).

add_library(DFDataLoggerIncludes INTERFACE)
target_include_directories(DFDataLoggerIncludes INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/models
)

set(DFDATALOGGER_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/DFDataLogger.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RepeatedLoggers.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DateAndTime.cpp
    ${CMAKE_CURRENT_LIST_DIR}/models/Deaths.cpp
)
set(DFDATALOGGER_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/DFDataLogger.hpp
    ${CMAKE_CURRENT_LIST_DIR}/models/Deaths.hpp
    ${CMAKE_CURRENT_LIST_DIR}/models/Events.hpp
    ${CMAKE_CURRENT_LIST_DIR}/models/Items.hpp
    ${CMAKE_CURRENT_LIST_DIR}/models/Jobs.hpp
    ${CMAKE_CURRENT_LIST_DIR}/models/Units.hpp
)



# Export lists to parent scope.
set(DFDATALOGGER_SOURCES ${DFDATALOGGER_SOURCES} PARENT_SCOPE)
set(DFDATALOGGER_HEADERS ${DFDATALOGGER_HEADERS} PARENT_SCOPE)


# Test app
add_executable(DateTimeTest test/DateTimeTest.cpp)
