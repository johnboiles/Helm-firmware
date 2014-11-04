#ifndef SeaTalkMessage_h
#define SeaTalkMessage_h
#include "inttypes.h"

typedef enum {
    SeaTalkMessageTypeWindAngle = 0x10,
    SeaTalkMessageTypeWindSpeed = 0x11,
    SeaTalkMessageTypeSpeedThroughWater = 0x20,
    SeaTalkMessageTypeDepth = 0x00,
    SeaTalkMessageTypeWaterTemperature = 0x23,
    SeaTalkMessageTypeDistanceDisplayUnits = 0x24,
} SeaTalkMessageType;


class BaseSeaTalkMessage
{
public:
    BaseSeaTalkMessage(const uint8_t *message, int messageLength);
    SeaTalkMessageType messageType();
    uint8_t *message();
    int messageLength();
    // TODO: These should be private but I forget how friend classes work in C++
    BaseSeaTalkMessage() {}
    uint8_t _message[19];
    int _messageLength;
};

class SeaTalkMessageWindAngle : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageWindAngle(const uint8_t *message, int messageLength) : BaseSeaTalkMessage(message, messageLength) {}
    float windAngle();
};

class SeaTalkMessageWindSpeed : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageWindSpeed(const uint8_t *message, int messageLength) : BaseSeaTalkMessage(message, messageLength) {}
    float windSpeed();
};

class SeaTalkMessageLatitude : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageLatitude(double latitude);
};

class SeaTalkMessageLongitude : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageLongitude(double longitude); 
};

class SeaTalkMessageTime : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageTime(int hour, int minute, int second);
};

BaseSeaTalkMessage *newSeaTalkMessage(const uint8_t *message, int messageLength);

void printSeaTalkMessage(uint8_t *message, int messageLength);

#endif
