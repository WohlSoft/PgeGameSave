#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch/catch.hpp>
#include <string>
#include <vector>

#include "../src/pge_gamesave_db.h"

TEST_CASE( "Dummy tester, does nothing", "[Dummy]" )
{
    PGE_GameSaveDB db;
    int64_t tempInteger;
    double tempDouble;
    std::string tempStr;

    REQUIRE( db.open("koko.savdb") );

    db.setEnvironment(PGE_GameSaveDB::ENV_LEVEL, "mylevel.lvl");

    REQUIRE( db.variableSet( PGE_GameSaveDB::VAR_ACCESS_THIS_LEVEL, "chicken", "koko", PGE_GameSaveDB::VTYPE_PLAIN_TEXT) );
    REQUIRE( db.variableGet( PGE_GameSaveDB::VAR_ACCESS_THIS_LEVEL, "chicken", &tempStr, "Nosense", PGE_GameSaveDB::VTYPE_PLAIN_TEXT) );
    REQUIRE( tempStr == "koko" );

    REQUIRE( db.variableSet( PGE_GameSaveDB::VAR_ACCESS_THIS_LEVEL, "chicken", "keke", PGE_GameSaveDB::VTYPE_PLAIN_TEXT) );

    REQUIRE( db.variableGet( PGE_GameSaveDB::VAR_ACCESS_THIS_LEVEL, "chicken", &tempStr, "Nosense", PGE_GameSaveDB::VTYPE_PLAIN_TEXT) );
    REQUIRE( tempStr == "keke" );

    REQUIRE( db.variableSet( PGE_GameSaveDB::VAR_ACCESS_THIS_LEVEL, "integer", (int64_t)324) );

    REQUIRE( db.variableSet( PGE_GameSaveDB::VAR_ACCESS_THIS_LEVEL, "QuestionOfEverything", (int64_t)42) );
    REQUIRE( db.variableSet( PGE_GameSaveDB::VAR_ACCESS_WORLD, "QuestionOfEverything", (int64_t)84) );

    REQUIRE( db.variableSet( PGE_GameSaveDB::VAR_ACCESS_THIS_LEVEL, "makeDouble", 45.1113342e1) );

    REQUIRE( db.variableGet( PGE_GameSaveDB::VAR_ACCESS_THIS_LEVEL, "integer", &tempInteger, (int64_t)-23) );
    REQUIRE( tempInteger == 324 );

    REQUIRE( db.variableGet( PGE_GameSaveDB::VAR_ACCESS_THIS_LEVEL, "QuestionOfEverything", &tempInteger, (int64_t)1) );
    REQUIRE( tempInteger == 42 );

    REQUIRE( db.variableGet( PGE_GameSaveDB::VAR_ACCESS_WORLD, "QuestionOfEverything", &tempInteger, (int64_t)1) );
    REQUIRE( tempInteger == 84 );

    REQUIRE( db.variableGet( PGE_GameSaveDB::VAR_ACCESS_THIS_LEVEL, "makeDouble", &tempDouble, -34.1) );
    REQUIRE( tempDouble == 45.1113342e1 );

    db.close();
}
