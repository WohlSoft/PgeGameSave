# This file downloads, configures and build sqlite.
#
# Output Variables:
# SQLITE_INSTALL_DIR  The install directory
# SQLITE_INCLUDE_DIR  The include directory of sqlite (.h)
# SQLITE_LIBRARY_DIR  The library directory of sqlite (.lib)

# Require ExternalProject and GIT!
include(ExternalProject)
find_package(Git REQUIRED)

# Posttible Input Vars:
# <None>

# SET OUTPUT VARS
set(SQLITE_INSTALL_DIR ${CMAKE_BINARY_DIR}/external/sqlite-install)
set(SQLITE_INCLUDE_DIR ${SQLITE_INSTALL_DIR}/include)
set(SQLITE_LIBRARY_DIR ${SQLITE_INSTALL_DIR}/lib)
set(SQLITE_LIBRARY_STATIC_FILES ${SQLITE_LIBRARY_DIR}/sqlite3$<CONFIG>.lib)

ExternalProject_Add(
    sqlite
    PREFIX ${CMAKE_BINARY_DIR}/external/sqlite
    GIT_REPOSITORY https://github.com/WohlSoft/SQLite3.git
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${SQLITE_INSTALL_DIR}"
)


