include_directories (${CMAKE_SOURCE_DIR}/src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR})  # TODO: DO as seperate component

add_executable (PGEGameSaveTest
                gamesave_test.cpp
                )
                
add_dependencies(PGEGameSaveTest PGEGameSaveLib)
target_link_libraries(PGEGameSaveTest PGEGameSaveLib sqlite3)

add_test (NAME PGEGameSaveTest COMMAND PGEGameSaveTest)



