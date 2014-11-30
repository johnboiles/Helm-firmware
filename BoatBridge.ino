#include <Arduino.h>
#include "NMEAParser.h"
#include "NMEAMessage.h"
#include "SeaTalkParser.h"
#include "SeaTalkMessage.h"
#include <AltSoftSerial.h>


#define DEBUG_LED LED_BUILTIN

// TX buffers for all the UARTs should be increased to make sure FIFO size is never a bottleneck. After all, the Teensy has 64k of RAM. Should be ok to make the TX buffers 200 bytes.
#define OUTPUT_SERIAL Serial
#define NMEA_HS_SERIAL Serial1
#define SEATALK_SERIAL Serial3
#define GPS_SERIAL Serial2
// NOTE: TX Buffer should be at least 160b (assuming GSV messages are dropped) since NMEA0183 is 4800 baud
AltSoftSerial NMEA_SERIAL;

NMEAParser GPS_PARSER;
NMEAParser AIS_PARSER;
NMEAParser INPUT_PARSER;
SeaTalkParser SEATALK_PARSER;

// TODO: I should make some sort of boat state object
float WindSpeed;
float WindAngle;

void setup() {
    cli();

    pinMode(DEBUG_LED, OUTPUT);

    // Teensy docs say "The baud rate is ignored and communication always occurs at full USB speed."
    OUTPUT_SERIAL.begin(115200);
    NMEA_HS_SERIAL.begin(38400);
    SEATALK_SERIAL.begin(4800, SERIAL_9N1_RXINV_TXINV);
    GPS_SERIAL.begin(9600);
    NMEA_SERIAL.begin(4800);

    // Enable interrupts
    sei();
}

#define SEND_SEATALK_MESSAGE(messageInstance) SEATALK_SERIAL.write9bit(messageInstance.message()[0] + 256); SEATALK_SERIAL.write(&(messageInstance.message()[1]), messageInstance.messageLength() - 1);
#define PRINT_SEATALK_MESSAGE(messageInstance) printSeaTalkMessage(messageInstance->message(), messageInstance->messageLength());

#define MESSAGE_IS_NMEA_TYPE(message, type) (message[3] == type[0] && message[4] == type[1] && message[5] == type[2])

void loop() {
    // Route AIS to the computer
    digitalWrite(DEBUG_LED, LOW);
    if (NMEA_HS_SERIAL.available()) {
        digitalWrite(DEBUG_LED, HIGH);
        uint8_t incomingByte = NMEA_HS_SERIAL.read();
        bool complete = AIS_PARSER.parse(incomingByte);
        if (complete) {
            OUTPUT_SERIAL.write(AIS_PARSER.message());
        }
    }
    // Route the GPS to the radio, computer, and SeaTalk network
    if (GPS_SERIAL.available()) {
        digitalWrite(DEBUG_LED, HIGH);
        uint8_t incomingByte = GPS_SERIAL.read();
        bool complete = GPS_PARSER.parse(incomingByte);
        if (complete) {
            const char *message = GPS_PARSER.message();
            OUTPUT_SERIAL.write(message);
            // Don't transmit unnecessary messages since NMEA_SERIAL's baud rate is lower
            if (!MESSAGE_IS_NMEA_TYPE(message, "GSV")) {
                NMEA_SERIAL.print(message);
            }
            NMEA_HS_SERIAL.write(message);
//            // Push location messages out over SeaTalk
//            if (MESSAGE_IS_NMEA_TYPE(message, "RMC")) {
//                NMEAMessageRMC rmc = NMEAMessageRMC(message);
//                SeaTalkMessageLongitude seaTalkMessageLongitude(rmc.longitude());
//                SEND_SEATALK_MESSAGE(seaTalkMessageLongitude);
//                SeaTalkMessageLatitude seaTalkMessageLatitude(rmc.latitude());
//                SEND_SEATALK_MESSAGE(seaTalkMessageLatitude);
//                SeaTalkMessageSpeedOverGround seaTalkMessageSpeedOverGround(rmc.speedOverGround());
//                SEND_SEATALK_MESSAGE(seaTalkMessageSpeedOverGround);
//                // This isn't quite the right translation. The SeaTalk message is magnetic course, and trackMadeGood is true course, but I don't think this should hurt anything
//                SeaTalkMessageMagneticCourse seaTalkMessageMagneticCourse(rmc.trackMadeGood().degrees);
//                SEND_SEATALK_MESSAGE(SeaTalkMessageMagneticCourse);
//                // Only send date once per minute
//                if (rmc.time().second == 0) {
//                    SeaTalkMessageDate seaTalkMessageDate(rmc.date());
//                    SEND_SEATALK_MESSAGE(seaTalkMessageDate);
//                    SeaTalkMessageMagneticVariation magneticVariation(-13);
//                    SEND_SEATALK_MESSAGE(magneticVariation);
//                }
//                // Send time every 10 seconds
//                if (((int)rmc.time().second) % 10 == 0) {
//                    SeaTalkMessageTime seaTalkMessageTime(rmc.time());
//                    SEND_SEATALK_MESSAGE(seaTalkMessageTime);
//                }
//            }
        }
    }
    if (OUTPUT_SERIAL.available()) {
        digitalWrite(DEBUG_LED, HIGH);
        // Consume incoming bytes. Teensy seems to crash otherwise
        uint8_t incomingByte = OUTPUT_SERIAL.read();
        bool complete = INPUT_PARSER.parse(incomingByte);
        if (complete) {
            const char *message = INPUT_PARSER.message();
            // Route APB and RMB info to the SeaTalk network
            // TODO: Detect conflicting route info coming in from the SeaTalk network and handle more gracefully
            if (MESSAGE_IS_NMEA_TYPE(message, "RMB")) {
                NMEAMessageRMB rmb = NMEAMessageRMB(message);
                SeaTalkMessageNavigationToWaypoint nav = SeaTalkMessageNavigationToWaypoint(rmb.xte(), rmb.bearingToDestination(), rmb.rangeToDestiation(), rmb.directionToSteer(), 0x7);
                SEND_SEATALK_MESSAGE(nav);
            } else if (MESSAGE_IS_NMEA_TYPE(message, "APB")) {
                NMEAMessageAPB apb = NMEAMessageAPB(message);
                SeaTalkMessageTargetWaypointName waypt = SeaTalkMessageTargetWaypointName(apb.destinationWaypointID());
                SEND_SEATALK_MESSAGE(waypt);
                if (apb.isArrived() || apb.isPerpendicualrPassed()) {
                    SeaTalkMessageArrivalInfo arr = SeaTalkMessageArrivalInfo(apb.isPerpendicualrPassed(), apb.isArrived(), apb.destinationWaypointID());
                    SEND_SEATALK_MESSAGE(arr);
                }
            }
        }
    }
    if (SEATALK_SERIAL.available()) {
        digitalWrite(DEBUG_LED, HIGH);
        int incomingByte = SEATALK_SERIAL.read();
        bool complete = SEATALK_PARSER.parse(incomingByte);
        if (complete) {
            // Create a message
            // TODO: Need to dig deeper into the UART so that I can do collision managment
            BaseSeaTalkMessage *message = newSeaTalkMessage(SEATALK_PARSER.message(), SEATALK_PARSER.messageLength());
            // PRINT_SEATALK_MESSAGE(message);
            SeaTalkMessageType messageType = message->messageType();
            if (messageType == SeaTalkMessageTypeWindAngle) {
                SeaTalkMessageWindAngle *windAngleMessage = (SeaTalkMessageWindAngle *)message;
                WindAngle = windAngleMessage->windAngle();
                NMEAMessageWind windMessage = NMEAMessageWind(WindAngle, WindSpeed);
                OUTPUT_SERIAL.print(windMessage.message());
            } else if (messageType == SeaTalkMessageTypeWindSpeed) {
                SeaTalkMessageWindSpeed *windSpeedMessage = (SeaTalkMessageWindSpeed *)message;
                WindSpeed = windSpeedMessage->windSpeed();
            } else if (messageType == SeaTalkMessageTypeDepth) {
                SeaTalkMessageDepth *depthMessage = (SeaTalkMessageDepth *)message;
                NMEAMessageDBT dbt = NMEAMessageDBT(depthMessage->depth());
                OUTPUT_SERIAL.print(dbt.message());
            } else if (messageType == SeaTalkMessageTypeSpeedThroughWater) {
                SeaTalkMessageSpeedThroughWater *speedMessage = (SeaTalkMessageSpeedThroughWater *)message;
                NMEAMessageVHW vhw = NMEAMessageVHW(speedMessage->speed());
                OUTPUT_SERIAL.print(vhw.message());
            } else if (messageType == SeaTalkMessageTypeCompassHeadingAndRudderPosition) {
                SeaTalkMessageCompassHeadingAndRudderPosition *headingMessage = (SeaTalkMessageCompassHeadingAndRudderPosition *)message;
                NMEAMessageHDM hdm = NMEAMessageHDM(headingMessage->compassHeading());
                OUTPUT_SERIAL.print(hdm.message());
            }
            delete[] message;
        }
    }
}
