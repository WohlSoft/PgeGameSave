# General Settings
set(PGE_GAME_SAVE_FILES 
    pge_gamesave_db.cpp
    pge_gamesave_db.h
)

# The Library
add_library (PGEGameSaveLib ${PGE_GAME_SAVE_FILES})

if(PGEGAMESAVE_DOWNLOAD_SQLITE_DEPENDENCY)
    add_dependencies(PGEGameSaveLib sqlite)
endif()

if(PGEGAMESAVE_CUSTOM_SQLITE_LIB AND PGEGAMESAVE_CUSTOM_SQLITE_INCLUDE_DIR)
    target_link_libraries(PGEGameSaveLib PGEGAMESAVE_CUSTOM_SQLITE_LIB)
endif()
