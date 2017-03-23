include_directories (${CMAKE_SOURCE_DIR}/src)

add_executable (PGEGameSaveTest
                gamesave_test.cpp
                )
                
add_dependencies(PGEGameSaveTest PGEGameSaveLib)
target_link_libraries(PGEGameSaveTest PGEGameSaveLib)

add_test (NAME PGEGameSaveTest COMMAND PGEGameSaveTest)



