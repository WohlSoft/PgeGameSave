
include_directories (${CMAKE_SOURCE_DIR}/test
                     ${CMAKE_SOURCE_DIR}/src
                    )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test/bin )

add_executable (GameSaveTest
                gamesave_test.cpp
                )
set_target_properties(GameSaveTest PROPERTIES COMPILE_DEFINITIONS "GSL_THROW_ON_CONTRACT_VIOLATION")
add_test (NAME GameSaveTest WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test/bin COMMAND GameSaveTest)

