#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../NMEAMessage.h"


TEST_CASE( "NMEAMessageRMC is parsed properly") {
	NMEAMessageRMC rmc = NMEAMessageRMC("$GPRMC,045431.00,A,3751.98405,N,12218.96980,W,0.078,,041114,,,D*68\r\n");
	float speedOverGround = rmc.speedOverGround();
    REQUIRE( speedOverGround == 0.078f );
}
