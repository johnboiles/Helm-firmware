#include "SeaTalkMessage.h"
#include <cstring>
#include <Arduino.h>
#include "math.h"


BaseSeaTalkMessage::BaseSeaTalkMessage(int messageLength) {
    _messageLength = messageLength;
}

BaseSeaTalkMessage::BaseSeaTalkMessage(const uint8_t *message, int messageLength) {
    _messageLength = messageLength;
    memcpy(_message, message, messageLength);
}

uint8_t *BaseSeaTalkMessage::message() {
    return _message;
}

SeaTalkMessageType BaseSeaTalkMessage::messageType() {
    return (SeaTalkMessageType)_message[0];
}

SeaTalkMessageDepth::SeaTalkMessageDepth(double depth) : BaseSeaTalkMessage(this->messageLength()) {
    // 00  02  YZ  XX XX  Depth below transducer: XXXX/10 feet 
    //  Flags in Y: Y&8 = 8: Anchor Alarm is active
    //             Y&4 = 4: Metric display units or
    //                      Fathom display units if followed by command 65
    //             Y&2 = 2: Used, unknown meaning
    // Flags in Z: Z&4 = 4: Transducer defective
    //             Z&2 = 2: Deep Alarm is active
    //             Z&1 = 1: Shallow Depth Alarm is active
    _message[0] = 0x00;
    _message[1] = 0x02;
    _message[2] = 0x00;
    int depthInteger = round(depth * 10);
    _message[3] = depthInteger & 0xFF;
    _message[4] = (depthInteger >> 8) & 0xFF;
}

float SeaTalkMessageWindAngle::windAngle() {
    // 10  01  XX  YY  Apparent Wind Angle: XXYY/2 degrees right of bow
    return ((_message[2] << 8) + _message[3]) / 2;
}

float SeaTalkMessageWindSpeed::windSpeed() {
    // 11  01  XX  0Y  Apparent Wind Speed: (XX & 0x7F) + Y/10 Knots
    return (_message[2] & 0x7F) + ((float)_message[3] / 10);
}

SeaTalkMessageSpeedThroughWater::SeaTalkMessageSpeedThroughWater(double speed) : BaseSeaTalkMessage(this->messageLength()) {
    // 20  01  XX  XX  Speed through water: XXXX/10 Knots 
    _message[0] = 0x20;
    _message[1] = 0x01;
    int intSpeed = round(speed * 10);
    _message[2] = intSpeed & 0xFF;
    _message[3] = (intSpeed >> 8) & 0xFF;
}

SeaTalkMessageLatitude::SeaTalkMessageLatitude(double latitude) : BaseSeaTalkMessage(this->messageLength()) {
    // 50  Z2  XX  YY  YY  LAT position: XX degrees, (YYYY & 0x7FFF)/100 minutes 
    // MSB of Y = YYYY & 0x8000 = South if set, North if cleared
    _message[0] = 0x50;
    _message[1] = 0x02;
    bool south = latitude < 0;
    latitude = fabs(latitude);
    int degrees = (int)latitude;
    _message[2] = degrees;
    int minutes = (latitude - degrees) * 60 * 100;
    _message[3] = minutes & 0xFF;
    _message[4] = (minutes & 0x7F00) >> 8;
    if (south) {
      _message[4] |= 0x80;
    }
}

SeaTalkMessageLongitude::SeaTalkMessageLongitude(double longitude) : BaseSeaTalkMessage(this->messageLength()) {
    // 51  Z2  XX  YY  YY  LON position: XX degrees, (YYYY & 0x7FFF)/100 minutes 
    // MSB of Y = YYYY & 0x8000 = East if set, West if cleared 
    _message[0] = 0x51;
    _message[1] = 0x02;
    bool east = longitude > 0;
    longitude = fabs(longitude);
    int degrees = (int)longitude;
    _message[2] = degrees;
    int minutes = (longitude - degrees) * 60 * 100;
    _message[3] = minutes & 0xFF;
    _message[4] = (minutes >> 8) & 0x7F;
    if (east) {
      _message[4] |= 0x80;
    }
}

SeaTalkMessageSpeedOverGround::SeaTalkMessageSpeedOverGround(double speed) : BaseSeaTalkMessage(this->messageLength()) {
    // 52  01  XX  XX  Speed over Ground: XXXX/10 Knots 
    _message[0] = 0x52;
    _message[1] = 0x01;
    int intSpeed = (speed * 10);
    _message[2] = intSpeed & 0xFF;
    _message[3] = (intSpeed >> 8) & 0xFF;
}

SeaTalkMessageMagneticCourse::SeaTalkMessageMagneticCourse(double course) : BaseSeaTalkMessage(this->messageLength()) {
    // 53  U0  VW      Magnetic Course in degrees: 
    // The two lower  bits of  U * 90 + 
    //    the six lower  bits of VW *  2 + 
    //    the two higher bits of  U /  2 = 
    //    (U & 0x3) * 90 + (VW & 0x3F) * 2 + (U & 0xC) / 8
    // The Magnetic Course may be offset by the Compass Variation (see datagram 99) to get the Course Over Ground (COG).
    // TODO: This message is crazy, I probably did it wrong
    _message[0] = 0x53;
    int quadrant = course / 90;
    _message[1] = (quadrant << 4);
    int degreesInQuadrant = (course - (quadrant * 90)) / 2;
    _message[2] = degreesInQuadrant & 0x3F;
    int fraction = (course - quadrant * 90 - degreesInQuadrant * 2) * 8;
    _message[1] |= (fraction << 6) & 0xC0;
}

SeaTalkMessageTime::SeaTalkMessageTime(Time time) : BaseSeaTalkMessage(this->messageLength()) {
    // 54  T1  RS  HH  GMT-time: HH hours, 
    // 6 MSBits of RST = minutes = (RS & 0xFC) / 4
    // 6 LSBits of RST = seconds =  ST & 0x3F 
    _message[0] = 0x54;
    int secondInteger = time.second;
    _message[1] = 0x01 | ((secondInteger & 0xF) << 4);
    _message[2] = 0xFC & (time.minute << 2);
    _message[2] |= 0x03 & ((secondInteger & 0x30) >> 4);
    _message[3] = time.hour;
}

BaseSeaTalkMessage *newSeaTalkMessage(const uint8_t *message, int messageLength) {
    // TODO: Assert that the message is the right length
    switch (message[1]) {
        case SeaTalkMessageTypeWindAngle:
            return new SeaTalkMessageWindAngle(message);
            break;
        case SeaTalkMessageTypeWindSpeed:
            return new SeaTalkMessageWindSpeed(message);
            break;
        case SeaTalkMessageTypeDepth:
            return new SeaTalkMessageDepth(message);
            break;
        case SeaTalkMessageTypeSpeedThroughWater:
            return new SeaTalkMessageSpeedThroughWater(message);
            break;
        default:
            return new BaseSeaTalkMessage(message, messageLength);
            break;
    }
}

void printSeaTalkMessage(uint8_t *message, int messageLength) {
    for (int i = 0; i < messageLength; i++) {
        if (i == 0) {
            Serial.printf("%03X ", message[i]);
        } else {
            Serial.printf("%02X ", message[i]);
        }
        if (i == (messageLength - 1)) {
             Serial.print("\r\n");
        }
    }
}
