message("Path to PGE Game Save is [${CMAKE_CURRENT_LIST_DIR}]")
include_directories(${CMAKE_CURRENT_LIST_DIR}/src)

set(PGE_GAMESAVE_SRCS)

list(APPEND PGE_GAMESAVE_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/src/pge_gamesave_db.cpp
)

