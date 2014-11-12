#include <Arduino.h>
#include "NMEAParser.h"
#include "NMEAMessage.h"
#include "SeaTalkParser.h"
#include "SeaTalkMessage.h"


#define DEBUG_LED LED_BUILTIN

#define OUTPUT_SERIAL Serial
#define NMEA_HS_SERIAL Serial1
#define SEATALK_SERIAL Serial3
#define GPS_SERIAL Serial2

NMEAParser GPS_PARSER;
NMEAParser AIS_PARSER;
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

    // Enable interrupts
    sei();
}

#define SEND_SEATALK_MESSAGE(messageInstance) SEATALK_SERIAL.write9bit(messageInstance.message()[0] + 256); SEATALK_SERIAL.write(&(messageInstance.message()[1]), messageInstance.messageLength() - 1);
#define PRINT_SEATALK_MESSAGE(messageInstance) printSeaTalkMessage(messageInstance->message(), messageInstance->messageLength());


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
            NMEA_HS_SERIAL.write(message);
            // Push location messages out over SeaTalk
            if (message[3] == 'R' && message[4] == 'M' && message[5] == 'C') {
                 NMEAMessageRMC rmc = NMEAMessageRMC(message);
                 SeaTalkMessageLongitude seaTalkMessageLongitude(rmc.longitude());
                 SEND_SEATALK_MESSAGE(seaTalkMessageLongitude);
                 SeaTalkMessageLatitude seaTalkMessageLatitude(rmc.latitude());
                 SEND_SEATALK_MESSAGE(seaTalkMessageLatitude);
                 SeaTalkMessageSpeedOverGround seaTalkMessageSpeedOverGround(rmc.speedOverGround());
                 SEND_SEATALK_MESSAGE(seaTalkMessageSpeedOverGround);
                 SeaTalkMessageTime seaTalkMessageTime(rmc.time());
                 SEND_SEATALK_MESSAGE(seaTalkMessageTime);
            }
        }
    }
    // if (OUTPUT_SERIAL.available()) {
    //     digitalWrite(DEBUG_LED, HIGH);
    //     uint8_t incomingByte = OUTPUT_SERIAL.read();
    //     // GPS_SERIAL.write(incomingByte);
    //     // NMEA_HS_SERIAL.write(incomingByte);
    //     SEATALK_SERIAL.write9bit(incomingByte + 256);
    // }
    while (SEATALK_SERIAL.available()) {
        digitalWrite(DEBUG_LED, HIGH);
        int incomingByte = SEATALK_SERIAL.read();
        bool complete = SEATALK_PARSER.parse(incomingByte);
        if (complete) {
            // Create a message
            // TODO: Need to dig deeper into the UART so that I can do collision managment
            BaseSeaTalkMessage *message = newSeaTalkMessage(SEATALK_PARSER.message(), SEATALK_PARSER.messageLength());
//            PRINT_SEATALK_MESSAGE(message);
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
            }
            delete[] message;
        }
    }
}
