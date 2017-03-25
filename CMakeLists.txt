cmake_minimum_required (VERSION 3.2)

project (PgeGameSave)

# Settings
option(PGEGAMESAVE_DOWNLOAD_SQLITE_DEPENDENCY "Downloads and compiles sqlite dependencies directly" OFF)
set(PGEGAMESAVE_CUSTOM_SQLITE_LIB "" CACHE FILEPATH "File to the sqlite dependency (if empty it will try to resolve it)")
set(PGEGAMESAVE_CUSTOM_SQLITE_INCLUDE_DIR "" CACHE PATH "Path to the include dir of sqlite")

if(NOT PGEGAMESAVE_CUSTOM_SQLITE_LIB OR NOT PGEGAMESAVE_CUSTOM_SQLITE_INCLUDE_DIR)
    # Try to resolve sqlite dependency
    if(PGEGAMESAVE_DOWNLOAD_SQLITE_DEPENDENCY)
        # Download and configure sqlite dependency
        include(cmake/DownloadSQLite.cmake)
        
        set(PGEGAMESAVE_CUSTOM_SQLITE_LIB ${SQLITE_LIBRARY_STATIC_FILES})
        set(PGEGAMESAVE_CUSTOM_SQLITE_INCLUDE_DIR ${SQLITE_INCLUDE_DIR})
    else()
        # Try to find sqlite dependency anyway
        find_package(SQLite3)
        
        # TODO: Add find_package script
    endif()
endif()

if(PGEGAMESAVE_CUSTOM_SQLITE_INCLUDE_DIR)
    include_directories(${PGEGAMESAVE_CUSTOM_SQLITE_INCLUDE_DIR})
endif()

# Enable Testing
enable_testing ()

# Set C++11
set(CMAKE_CXX_STANDARD 11)

# Add library
add_subdirectory(src)

# Add test
add_subdirectory(test)

