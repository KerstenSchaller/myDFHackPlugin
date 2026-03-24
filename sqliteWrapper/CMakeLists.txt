cmake_minimum_required(VERSION 3.10)
project(MySQLiteApp)

set(CMAKE_CXX_STANDARD 17)

# Add SQLite source
add_library(sqlite3 third_party/sqlite/sqlite3.c)

# header only
add_library(DB INTERFACE SQLiteWrapper.hpp)
target_include_directories(DB INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite_modern_cpp/hdr)
target_include_directories(DB INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(DB INTERFACE sqlite3)

# Include directories
target_include_directories(DB INTERFACE
    third_party/sqlite
    third_party/sqlite_modern_cpp/hdr
)

# Your executable
add_executable(my_sqlite_wrapper_test main.cpp)

# Link SQLite
target_link_libraries(my_sqlite_wrapper_test 
    DB)