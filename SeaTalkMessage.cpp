#include "SeaTalkMessage.h"
#include <cstring>
#include <Arduino.h>

BaseSeaTalkMessage::BaseSeaTalkMessage(const uint8_t *message, int messageLength) {
    memcpy(_message, message, messageLength);
    _messageLength = messageLength;
}

uint8_t *BaseSeaTalkMessage::message() {
    return _message;
}

int BaseSeaTalkMessage::messageLength() {
    return _messageLength;
}

SeaTalkMessageType BaseSeaTalkMessage::messageType() {
    return (SeaTalkMessageType)_message[0];
}

float SeaTalkMessageWindAngle::windAngle() {
    // 10  01  XX  YY  Apparent Wind Angle: XXYY/2 degrees right of bow
    return ((_message[2] << 8) + _message[3]) / 2;
}

float SeaTalkMessageWindSpeed::windSpeed() {
    // 11  01  XX  0Y  Apparent Wind Speed: (XX & 0x7F) + Y/10 Knots
    return (_message[2] & 0x7F) + ((float)_message[3] / 10);
}

SeaTalkMessageLatitude::SeaTalkMessageLatitude(double latitude) {
    // 50  Z2  XX  YY  YY  LAT position: XX degrees, (YYYY & 0x7FFF)/100 minutes 
    // MSB of Y = YYYY & 0x8000 = South if set, North if cleared
    _message[0] = 0x50;
    _message[1] = 0x02;
    bool south = latitude < 0;
    latitude = fabs(latitude);
    int degrees = (int)latitude;
    _message[2] = degrees;
    int minutes = (latitude - degrees) * 60 * 100;
    _message[4] = (minutes & 0x7F00) >> 8;
    if (south) {
      _message[4] |= 0x80;
    }
    _message[3] = minutes & 0xFF;
    _messageLength = 5;
}

SeaTalkMessageLongitude::SeaTalkMessageLongitude(double longitude) {
    // 51  Z2  XX  YY  YY  LON position: XX degrees, (YYYY & 0x7FFF)/100 minutes 
    // MSB of Y = YYYY & 0x8000 = East if set, West if cleared 
    _message[0] = 0x51;
    _message[1] = 0x02;
    bool east = longitude > 0;
    longitude = fabs(longitude);
    int degrees = (int)longitude;
    _message[2] = degrees;
    int minutes = (longitude - degrees) * 60 * 100;
    _message[4] = (minutes >> 8) & 0x7F;
    if (east) {
      _message[4] |= 0x80;
    }
    _message[3] = minutes & 0xFF;
    _messageLength = 5;
}

SeaTalkMessageTime::SeaTalkMessageTime(int hour, int minute, int second) {
    // 54  T1  RS  HH  GMT-time: HH hours, 
    // 6 MSBits of RST = minutes = (RS & 0xFC) / 4
    // 6 LSBits of RST = seconds =  ST & 0x3F 
    _message[0] = 0x54;
    _message[1] = 0x01 | ((second & 0xF) << 4);
    _message[2] = 0xFC & (minute << 2);
    _message[2] |= 0x03 & ((second & 0x30) >> 4);
    _message[3] = hour;
    _messageLength = 4;
}

BaseSeaTalkMessage *newSeaTalkMessage(const uint8_t *message, int messageLength) {
    switch (message[1]) {
        case SeaTalkMessageTypeWindAngle:
            return new SeaTalkMessageWindAngle(message, messageLength);
            break;
        case SeaTalkMessageTypeWindSpeed:
            return new SeaTalkMessageWindSpeed(message, messageLength);
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

