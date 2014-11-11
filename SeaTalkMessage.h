#ifndef SeaTalkMessage_h
#define SeaTalkMessage_h

#include "inttypes.h"
#include "Types.h"

typedef enum {
    SeaTalkMessageTypeWindAngle = 0x10,
    SeaTalkMessageTypeWindSpeed = 0x11,
    SeaTalkMessageTypeSpeedThroughWater = 0x20,
    SeaTalkMessageTypeDepth = 0x00,
    SeaTalkMessageTypeWaterTemperature = 0x23,
    SeaTalkMessageTypeDistanceDisplayUnits = 0x24,
    SeaTalkMessageTypeLatitude = 0x50,
    SeaTalkMessageTypeLongitude = 0x51,
    SeaTalkMessageTypeSpeedOverGround = 0x52,
    SeaTalkMessageTypeMagneticCourse = 0x53,
    SeaTalkMessageTypeTime = 0x54
} SeaTalkMessageType;


class BaseSeaTalkMessage
{
public:
    BaseSeaTalkMessage(const uint8_t *message, int messageLength);
    BaseSeaTalkMessage(int messageLength);
    SeaTalkMessageType messageType();
    uint8_t *message();
    int messageLength() { return _messageLength; }
    // TODO: These should be private but I forget how friend classes work in C++
    int _messageLength;
    uint8_t _message[18];
};

class SeaTalkMessageDepth : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageDepth(const uint8_t *message) : BaseSeaTalkMessage(message, this->messageLength()) {}
    SeaTalkMessageDepth(double depth);
    int messageLength() { return 5; }
    //! Depth in feet
    double depth() { return (_message[3] + (_message[4] << 8)) / 10.0; }
    bool isAnchorAlarmActive() { return !!(_message[2] & 0x80); }
    bool isMetricDisplayUnits() { return !!(_message[2] & 0x40); }
    bool isTransducerDefective() { return !!(_message[2] & 0x04); }
    bool isDeepAlarmActive() { return !!(_message[2] & 0x02); }
    bool isShallowAlarmActive() { return !!(_message[2] & 0x01); }
};

class SeaTalkMessageWindAngle : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageWindAngle(const uint8_t *message) : BaseSeaTalkMessage(message, this->messageLength()) {}
    int messageLength() { return 4; }
    float windAngle();
};

class SeaTalkMessageWindSpeed : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageWindSpeed(const uint8_t *message) : BaseSeaTalkMessage(message, this->messageLength()) {}
    int messageLength() { return 4; }
    float windSpeed();
};

class SeaTalkMessageSpeedThroughWater : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageSpeedThroughWater(const uint8_t *message) : BaseSeaTalkMessage(message, this->messageLength()) {}
    SeaTalkMessageSpeedThroughWater(double speed);
    int messageLength() { return 4; }
    //! Speed in knots
    double speed() { return (_message[2] + (_message[3] << 8)) / 10.0; }
};

class SeaTalkMessageLatitude : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageLatitude(double latitude);
    int messageLength() { return 5; }
};

class SeaTalkMessageLongitude : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageLongitude(double longitude); 
    int messageLength() { return 5; }
};

class SeaTalkMessageSpeedOverGround : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageSpeedOverGround(double speed);
    int messageLength() { return 4; }
};

class SeaTalkMessageMagneticCourse : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageMagneticCourse(double course);
    int messageLength() { return 3; }
};

class SeaTalkMessageTime : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageTime(Time time);
    int messageLength() { return 4; }
};

BaseSeaTalkMessage *newSeaTalkMessage(const uint8_t *message, int messageLength);

void printSeaTalkMessage(uint8_t *message, int messageLength);

#endif
