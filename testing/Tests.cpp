#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../NMEAMessage.h"
#include "../SeaTalkMessage.h"
#include "../SeaTalkParser.h"


void failForDifferingArrays(uint8_t *array, uint8_t *expectedArray, int length, const char *failureMessage) {
    char messageString[55];
    char expectedString[55];
    for (int i = 0; i < length; i++) {
        sprintf(&messageString[i * 3], "%02x ", array[i]);
        sprintf(&expectedString[i * 3], "%02x ", expectedArray[i]);
    }
    messageString[length * 3 + 1] = 0;
    expectedString[length * 3 + 1] = 0;
    FAIL(failureMessage << "\r\nActual  : " << messageString << "\r\nExpected: " <<  expectedString);
}

bool arraysAreEqual(uint8_t *array, uint8_t *expectedArray, int length) {
    for (int i = 0; i < length; i++) {
        if ( array[i] != expectedArray[i] ) {
            return false;
        }
    }
    return true;
}


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
    REQUIRE( rmc.trackMadeGood().degrees == 45.2f );
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

TEST_CASE( "NMEAMessageDBT is constructed properly" ) {
    NMEAMessageDBT dbt = NMEAMessageDBT(24.3);
    REQUIRE( strlen(dbt.message()) == 24 );
    REQUIRE( std::string(dbt.message()) == std::string("$STDBT,24.3,f,,M,,F*23\r\n") );
}

TEST_CASE( "NMEAMessageVHW is constructed properly" ) {
    NMEAMessageVHW vhw = NMEAMessageVHW(6.39);
    REQUIRE( strlen(vhw.message()) == 26);
    REQUIRE( std::string(vhw.message()) == std::string("$STVHW,,T,,M,6.4,N,,K*7E\r\n") );
}

TEST_CASE( "NMEAMessageHDM is constructed properly" ) {
    NMEAMessageHDM hdm = NMEAMessageHDM(236.3);
    REQUIRE( strlen(hdm.message()) == 19);
    REQUIRE( std::string(hdm.message()) == std::string("$STHDM,236.3,M*21\r\n") );
}

TEST_CASE( "NMEAMessageRMB is parsed properly" ) {
    NMEAMessageRMB rmb = NMEAMessageRMB("$ECRMB,A,0.000,L,tospace,001,3751.944,N,12219.721,W,0.596,266.197,0.055,V*37\r\n");
    REQUIRE( rmb.status() == StatusActive );
    REQUIRE( rmb.xte() == 0.0 );
    REQUIRE( rmb.directionToSteer() == LateralityLeft );
    REQUIRE( rmb.toWaypointID() == std::string("tospace") );
    REQUIRE( rmb.fromWaypointID() == std::string("001") );
    REQUIRE( rmb.destinationLatitude() == 37.86573333333333333);
    REQUIRE( rmb.destinationLongitude() == -122.32868333333333);
    REQUIRE( rmb.rangeToDestiation() == 0.596f );
    REQUIRE( rmb.bearingToDestination().degrees == 266.197f);
    REQUIRE( rmb.destinationClosingVelocity() == 0.055f);
}

TEST_CASE( "NMEAMessageAPB is parsed properly" ) {
    NMEAMessageAPB apb = NMEAMessageAPB("$ECAPB,A,A,2.345,L,N,V,V,266.243,T,001,266.197,T,266.197,T*35");
    REQUIRE( apb.isUnreliableFix() == false );
    REQUIRE( apb.isCycleLockWarning() == false );
    REQUIRE( apb.xte() == 2.345f );
    REQUIRE( apb.directionToSteer() == LateralityLeft );
    REQUIRE( apb.isArrived() == false );
    REQUIRE( apb.isPerpendicularPassed() == false );
    REQUIRE( apb.bearingOriginToDestination().degrees == 266.243f );
    REQUIRE( apb.bearingOriginToDestination().isMagnetic == false );
    REQUIRE( apb.destinationWaypointID() == std::string("001") );
    REQUIRE( apb.bearingPresentToDestination().degrees == 266.197f );
    REQUIRE( apb.bearingPresentToDestination().isMagnetic == false );
    REQUIRE( apb.headingToSteerToWaypoint().degrees == 266.197f );
    REQUIRE( apb.headingToSteerToWaypoint().isMagnetic == false );
}

TEST_CASE( "NMEAMessageSEA is constructed properly" ) {
    uint8_t message[4] = {0x10, 0x11, 0x02, 0x6E};
    NMEAMessageSEA sea = NMEAMessageSEA(message, 4);
    REQUIRE( std::string(sea.message()) == std::string("$STSEA,1011026E*0C\r\n") );
}

TEST_CASE( "NMEAMessageSEA is parsed properly" ) {
    uint8_t message[4] = {0x10, 0x11, 0x02, 0x6E};
    NMEAMessageSEA sea = NMEAMessageSEA("$STSEA,1011026E*0C\r\n");
    REQUIRE ( sea.seaTalkMessageLength() == 4 );
    if (!arraysAreEqual(sea.seaTalkMessage(), message, 4)) {
        failForDifferingArrays(sea.seaTalkMessage(), message, 4, "NMEAMessageSEA generated incorrect byte string.");
    }
}

TEST_CASE( "SeaTalkParser" ) {
    SeaTalkParser parser = SeaTalkParser();
    bool completed;
    completed = parser.parse(0x199);
    REQUIRE( completed == false );
    completed = parser.parse(0x00);
    REQUIRE( completed == false );
    completed = parser.parse(0xF3);
    REQUIRE( completed == true );
    REQUIRE( parser.messageLength() == 3 );
}

TEST_CASE( "SeaTalkMessageWindAngle is parsed properly" ) {
    uint8_t message[4] = {0x10, 0x11, 0x02, 0x6E};
    SeaTalkMessageWindAngle windAngle = SeaTalkMessageWindAngle(message);
    REQUIRE( windAngle.windAngle() == 311.0 );
}

TEST_CASE( "SeaTalkMessageWindSpeed is parsed properly" ) {
    uint8_t message[4] = {0x11, 0x11, 0x02, 0x03};
    SeaTalkMessageWindSpeed windSpeed = SeaTalkMessageWindSpeed(message);
    REQUIRE( windSpeed.windSpeed() == 2.3f );
}

void assertEqualSeaTalkMessages(BaseSeaTalkMessage *seaTalkMessage, uint8_t *expected, int expectedLength) {
    REQUIRE ( seaTalkMessage->messageLength() == expectedLength );
    bool failed = !arraysAreEqual(seaTalkMessage->message(), expected, expectedLength);
    
    if (failed) {
        failForDifferingArrays(seaTalkMessage->message(), expected, expectedLength, "SeaTalk messages differ");
    }
}

TEST_CASE( "SeaTalkMessageLampIntensity is generated properly ") {
    SeaTalkMessageLampIntensity lampIntensity = SeaTalkMessageLampIntensity(3);
    uint8_t expected[3] = {0x30, 0x00, 0x0C};
    assertEqualSeaTalkMessages(&lampIntensity, expected, 3);
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
    SeaTalkMessageMagneticCourse mc = SeaTalkMessageMagneticCourse(149.9);
    uint8_t expected[3] = {0x53, 0x10, 0x1e};
    assertEqualSeaTalkMessages(&mc, expected, sizeof(expected));
}

TEST_CASE( "SeaTalkMessageMagneticCourse rounding" ) {
    SeaTalkMessageMagneticCourse mc = SeaTalkMessageMagneticCourse(159.9);
    REQUIRE( mc.course() == 160.0f );

    mc = SeaTalkMessageMagneticCourse(149.9);
    REQUIRE( mc.course() == 150.0f );

    mc = SeaTalkMessageMagneticCourse(149.6);
    REQUIRE( mc.course() == 149.5f );

    mc = SeaTalkMessageMagneticCourse(249.4);
    REQUIRE( mc.course() == 249.5f );

    mc = SeaTalkMessageMagneticCourse(349.2);
    REQUIRE( mc.course() == 349.0f );
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

TEST_CASE( "SeaTalkMessageDepth is generated properly" ) {
    SeaTalkMessageDepth depth = SeaTalkMessageDepth(12.36);
    uint8_t expected[5] = {0x00, 0x02, 0x00, 0x7C, 0x00};
    assertEqualSeaTalkMessages(&depth, expected, 5);
}

TEST_CASE( "SeaTalkMessageDepth is parsed properly" ) {
    uint8_t message[5] = {0x00, 0x42, 0x30, 0x46, 0x05};
    SeaTalkMessageDepth depth = SeaTalkMessageDepth(message);
    REQUIRE( depth.depth() == 135 );
}

TEST_CASE( "SeaTalkMessageSpeedThroughWater is generated properly" ) {
    SeaTalkMessageSpeedThroughWater stw = SeaTalkMessageSpeedThroughWater(5.3);
    uint8_t expected[4] = {0x20, 0x01, 0x35, 0x00};
    assertEqualSeaTalkMessages(&stw, expected, 4);
}

TEST_CASE( "SeaTalkMessageSpeedThroughWater is parsed properly" ) {
    uint8_t message[4] = {0x20, 0x41, 0x35, 0x00};
    SeaTalkMessageSpeedThroughWater stw = SeaTalkMessageSpeedThroughWater(message);
    REQUIRE( stw.speed() == 5.3 );
}

TEST_CASE( "SeaTalkMessageTargetWaypointName is parsed properly" ) {
    uint8_t message[8] = {0x82, 0x05, 0x00, 0xFF, 0x00, 0xFF, 0x04, 0xFB};
    SeaTalkMessageTargetWaypointName twn = SeaTalkMessageTargetWaypointName(message);
    REQUIRE( twn.name() == std::string("0001") );
}

TEST_CASE( "SeaTalkMessageTargetWaypointName is generated properly" ) {
    uint8_t expected[8] = {0x82, 0x05, 0x00, 0xFF, 0x00, 0xFF, 0x04, 0xFB};
    SeaTalkMessageTargetWaypointName twn = SeaTalkMessageTargetWaypointName("0001");
    assertEqualSeaTalkMessages(&twn, expected, sizeof(expected));
}

TEST_CASE( "SeaTalkMessageTargetWaypointName loopback test" ) {
    SeaTalkMessageTargetWaypointName twn = SeaTalkMessageTargetWaypointName("asDf");
    SeaTalkMessageTargetWaypointName twn2 = SeaTalkMessageTargetWaypointName(twn.message());
    REQUIRE( twn2.name() == std::string("ASDF") );
}

TEST_CASE( "SeaTalkMessageCompassHeadingAutopilotCourseRudderPosition is parsed properly" ) {
    // Actual data from ST1000
    uint8_t message[9] = {0x84, 0xA6, 0x1C, 0x00, 0x04, 0x00, 0xFC, 0x00, 0x08};
    SeaTalkMessageCompassHeadingAutopilotCourseRudderPosition apinfo = SeaTalkMessageCompassHeadingAutopilotCourseRudderPosition(message);
    REQUIRE( apinfo.compassHeading() == 237 );
    REQUIRE( apinfo.autopilotCourse() == 0 );
    REQUIRE( apinfo.isVaneMode() == true );
    REQUIRE( apinfo.isAutoMode() == false );
}

TEST_CASE( "SeaTalkMessageNavigationToWaypoint is generated properly" ) {
    // Created from the examples in the Knauf doc
    uint8_t expected[9] = {0x85, 0x56, 0x10, 0x42, 0x16, 0x20, 0x17, 0x00, 0xE8};
    Heading bearingToDestination;
    bearingToDestination.degrees = 230;
    bearingToDestination.isMagnetic = true;
    SeaTalkMessageNavigationToWaypoint nav = SeaTalkMessageNavigationToWaypoint(2.61, bearingToDestination, 5.13, LateralityLeft, 0x7);
    assertEqualSeaTalkMessages(&nav, expected, sizeof(expected));
}

TEST_CASE( "SeaTalkMessageNavigationToWaypoint is parsed properly" ) {
    // Created from the examples in the Knauf doc
    uint8_t message[9] = {0x85, 0x56, 0x10, 0x42, 0x16, 0x20, 0x17, 0x00, 0xE8};
    SeaTalkMessageNavigationToWaypoint nav = SeaTalkMessageNavigationToWaypoint(message);
    REQUIRE( nav.xte() == 2.61f );
    REQUIRE( nav.bearingToDestination().degrees == 230 );
    REQUIRE( nav.bearingToDestination().isMagnetic == true );
    REQUIRE( nav.distanceToDestination() == 5.13f );
    REQUIRE( nav.directionToSteer() == LateralityLeft );
    REQUIRE( nav.trackControlMode() == 0x7 );
}

TEST_CASE( "SeaTalkMessageSetAutopilotParameter loopback test ") {
    uint8_t expected[5] = {0x92, 0x02, 0x0C, 0x0E, 0x00};
    SeaTalkMessageSetAutopilotParameter apParam = SeaTalkMessageSetAutopilotParameter(0xC, 14);
    assertEqualSeaTalkMessages(&apParam, expected, 5);
    SeaTalkMessageSetAutopilotParameter apParam2 = SeaTalkMessageSetAutopilotParameter(apParam.message());
    REQUIRE( apParam2.parameterNumber() == 0xC );
    REQUIRE( apParam2.parameterValue() == 14 );
}

TEST_CASE( "SeaTalkMessageMagneticVariation is generated properly" ) {
    uint8_t expected[3] = {0x99, 0x00, 0xF3};
    SeaTalkMessageMagneticVariation mv = SeaTalkMessageMagneticVariation(-13);
    assertEqualSeaTalkMessages(&mv, expected, sizeof(expected));
}

TEST_CASE( "SeaTalkMessageCompassHeadingAndRudderPosition is parsed properly" ) {
    uint8_t message[4] = {0x9C, 0xA1, 0x1C, 0xFC};
    SeaTalkMessageCompassHeadingAndRudderPosition heading = SeaTalkMessageCompassHeadingAndRudderPosition(message);
    REQUIRE( heading.compassHeading() == 237 );
}

TEST_CASE( "SeaTalkMessageCompassHeadingAndRudderPosition is generated properly" ) {
    uint8_t expected[4] = {0x9C, 0xA1, 0x1C, 0x00};
    SeaTalkMessageCompassHeadingAndRudderPosition heading = SeaTalkMessageCompassHeadingAndRudderPosition(237, false, 0);
    assertEqualSeaTalkMessages(&heading, expected, sizeof(expected));
}

TEST_CASE( "SeaTalkMessageArrivalInfo is generated properly" ) {
    uint8_t expected[7] = {0xA2, 0x44, 0x00, 0, '0', '0', '1'};
    SeaTalkMessageArrivalInfo arr = SeaTalkMessageArrivalInfo(false, true, "001");
    assertEqualSeaTalkMessages(&arr, expected, sizeof(expected));
}

TEST_CASE( "SeaTalkMessageArrivalInfo only uses the last 4 of name" ) {
    uint8_t expected[7] = {0xA2, 0x44, 0x00, 'U', 'P', 'E', 'R'};
    SeaTalkMessageArrivalInfo arr = SeaTalkMessageArrivalInfo(false, true, "superduper");
    assertEqualSeaTalkMessages(&arr, expected, sizeof(expected));
}

TEST_CASE( "SeaTalkMessageDeviceQuery is generated properly" ) {
    uint8_t expected[5] = {0xA4, 0x02, 0x00, 0x00, 0x00};
    SeaTalkMessageDeviceQuery dq = SeaTalkMessageDeviceQuery();
    assertEqualSeaTalkMessages(&dq, expected, sizeof(expected));
}
