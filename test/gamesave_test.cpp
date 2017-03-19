#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch/catch.hpp>
#include <string>
#include <vector>

TEST_CASE( "Dummy tester, does nothing", "[Dummy]" )
{
    REQUIRE( true );
}
