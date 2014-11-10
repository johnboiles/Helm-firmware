#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../NMEAMessage.h"
#include "../SeaTalkMessage.h"


TEST_CASE( "NMEAMessageRMC is parsed properly" ) {
    // Actual message from a uBlox-6 GPS
    NMEAMessageRMC rmc = NMEAMessageRMC("$GPRMC,045431.00,A,3751.98405,N,12218.96980,W,0.078,,041114,,,D*68\r\n");
    REQUIRE( rmc.time().hour == 4 );
    REQUIRE( rmc.time().minute == 54 );
    REQUIRE( rmc.time().second == 31 );
    REQUIRE( rmc.latitude() == 37.86640083333333 );
    REQUIRE( rmc.longitude() == -122.31616333333334 );
    REQUIRE( rmc.speedOverGround() == 0.078 );
    REQUIRE( rmc.date().day == 4 );
    REQUIRE( rmc.date().month == 11 );
    REQUIRE( rmc.date().year == 14 );

    // Theoretical message including track made good and magnetic variation
    rmc = NMEAMessageRMC("$GPRMC,045431.00,A,3751.98405,N,12218.96980,W,0.078,45.2,041114,42.2,W,D*68\r\n");
    REQUIRE( rmc.trackMadeGood() == 45.2 );
    REQUIRE( rmc.magneticVariation() == -42.2 );
}

TEST_CASE( "NMEAMessageGLL is paresd properly" ) {
    // Actual message from a uBlox-6 GPS
    NMEAMessageGLL gll = NMEAMessageGLL("$GPGLL,3751.98415,N,12218.97005,W,045445.00,A,D*78\r\n");
    REQUIRE( gll.latitude() == 37.8664025 );
    REQUIRE( gll.longitude() == -122.3161675 );
    REQUIRE( gll.time().hour == 4 );
    REQUIRE( gll.time().minute == 54 );
    REQUIRE( gll.time().second == 45 );
}

TEST_CASE( "NMEAMessageWind is constructed properly" ) {
    NMEAMessageWind mwv = NMEAMessageWind(315.4, 12.2);
    REQUIRE( strlen(mwv.message()) == 28 );
    // Comparisons of char* don't work
    REQUIRE( std::string(mwv.message()) == std::string("$WIMWV,315.4,R,12.2,N,A*11\r\n") );
}

TEST_CASE( "SeaTalkMessageWindAngle is parsed properly" ) {
    uint8_t message[4] = {0x10, 0x11, 0x02, 0x6E};
    SeaTalkMessageWindAngle windAngle = SeaTalkMessageWindAngle(message, 4);
    REQUIRE( windAngle.windAngle() == 311.0 );
}

TEST_CASE( "SeaTalkMessageWindSpeed is parsed properly" ) {
    uint8_t message[4] = {0x11, 0x11, 0x02, 0x03};
    SeaTalkMessageWindSpeed windSpeed = SeaTalkMessageWindSpeed(message, 4);
    REQUIRE( windSpeed.windSpeed() == 2.3f );
}

void assertEqualSeaTalkMessages(BaseSeaTalkMessage *seaTalkMessage, uint8_t *expected, int expectedLength) {
    REQUIRE ( seaTalkMessage->messageLength() == expectedLength );
    bool failed = false;
    for (int i = 0; i < seaTalkMessage->messageLength(); i++) {
        if ( seaTalkMessage->message()[i] != expected[i] ) {
            failed = true;
        }
    }
    if (failed) {
        char messageString[55];
        char expectedString[55];
        for (int i = 0; i < expectedLength; i++) {
            sprintf(&messageString[i * 3], "%02x ", seaTalkMessage->message()[i]);
            sprintf(&expectedString[i * 3], "%02x ", expected[i]);
        }
        messageString[expectedLength * 3 + 1] = 0;
        expectedString[expectedLength * 3 + 1] = 0;
        FAIL("SeaTalk messages differ\r\nActual  : " << messageString << "\r\nExpected: " <<  expectedString);
    }
}

TEST_CASE( "SeaTalkMessageLatitude is generated properly" ) {
    SeaTalkMessageLatitude lat = SeaTalkMessageLatitude(37.866384);
    uint8_t expected[5] = {0x50, 0x02, 0x25, 0x4E, 0x14};
    assertEqualSeaTalkMessages(&lat, expected, 5);
    //$GPRMC,045547.00,A,3751.98304,N,12218.97136,W,0.016,,041114,,,D*62
    //050 62 25 4E 14
}

TEST_CASE( "SeaTalkMessageLongitude is generated properly" ) {
    SeaTalkMessageLongitude lon = SeaTalkMessageLongitude(-122.316189333);
    uint8_t expected[5] = {0x51, 0x02, 0x7A, 0x69, 0x07};
    assertEqualSeaTalkMessages(&lon, expected, 5);
    //$GPRMC,045547.00,A,3751.98304,N,12218.97136,W,0.016,,041114,,,D*62
    //050 62 25 4E 14
}

TEST_CASE( "SeaTalkMessageSpeedOverGround is generated properly" ) {
    // Not based on actual data
    SeaTalkMessageSpeedOverGround sog = SeaTalkMessageSpeedOverGround(0);
    uint8_t expected[4] = {0x52, 0x01, 0x00, 0x00};
    assertEqualSeaTalkMessages(&sog, expected, 4);
}

TEST_CASE( "SeaTalkMessageMagneticCourse is generated properly" ) {
    // Not based on actual data
    SeaTalkMessageMagneticCourse mc = SeaTalkMessageMagneticCourse(0);
    uint8_t expected[3] = {0x53, 0x00, 0x00};
    assertEqualSeaTalkMessages(&mc, expected, 3);
}

TEST_CASE( "SeaTalkMessageTime is generated properly" ) {
    // Here's the NMEA message and how the RayMarine chartplotter translates its time. Hours seem to set to the local timezone. However, Thomas Knauf says the SeaTalk message is supposedly for GMT.
    // $GPRMC,045443.00,A,3751.98419,N,12218.97005,W,0.030,,041114,,,D*69
    // 054 71 D9 16
    Time time;
    time.hour = 4;
    time.minute = 54;
    time.second = 43;
    SeaTalkMessageTime sttime = SeaTalkMessageTime(time);
    uint8_t expected[4] = {0x54, 0xB1, 0xDA, 0x04};
    assertEqualSeaTalkMessages(&sttime, expected, 4);
}
