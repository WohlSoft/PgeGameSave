# General Settings
set(PGE_GAME_SAVE_FILES 
    pge_gamesave_db.cpp
    pge_gamesave_db.h
)

# The Library
add_library (PGEGameSaveLib ${PGE_GAME_SAVE_FILES})

