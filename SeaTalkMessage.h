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
    SeaTalkMessageTypeLampIntensity = 0x30,
    SeaTalkMessageTypeLatitude = 0x50,
    SeaTalkMessageTypeLongitude = 0x51,
    SeaTalkMessageTypeSpeedOverGround = 0x52,
    SeaTalkMessageTypeMagneticCourse = 0x53,
    SeaTalkMessageTypeTime = 0x54,
    SeaTalkMessageTypeCompassHeadingAutopilotCourseRudderPosition = 0x84,
    SeaTalkMessageTypeCompassHeadingAndRudderPosition = 0x9C,
    SeaTalkMessageTypeDeviceQuery = 0xA4
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

class SeaTalkMessageWaterTemperature : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageWaterTemperature(const uint8_t *message) : BaseSeaTalkMessage(message, this->messageLength()) {}
    SeaTalkMessageWaterTemperature(int celcius);
    int messageLength() { return 4; }
    bool invalid() { return (_message[1] & 0x40) == 0x40; }
    int temperatureCelcius() { return _message[2]; }
    int temperatureFarenheit() { return _message[3]; }
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

class SeaTalkMessageLampIntensity : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageLampIntensity(const uint8_t *message) : BaseSeaTalkMessage(message, this->messageLength()) {}
    SeaTalkMessageLampIntensity(uint8_t intensity);
    int messageLength() { return 3; }
    //! Intensity 0-3
    uint8_t intensity() { return (_message[2] & 0xF) / 4; }
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

class SeaTalkMessageDate : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageDate(const uint8_t *message) : BaseSeaTalkMessage(message, this->messageLength()) {}
    SeaTalkMessageDate(Date date);
    int messageLength() { return 4; }
    Date date();
};

// 84  U6  VW  XY 0Z 0M RR SS TT  Compass heading  Autopilot course and 
// Rudder position (see also command 9C) 
// Compass heading in degrees: 
//   The two lower  bits of  U * 90 + 
//   the six lower  bits of VW *  2 + 
//   number of bits set in the two higher bits of U = 
//   (U & 0x3)* 90 + (VW & 0x3F)* 2 + (U & 0xC ? (U & 0xC == 0xC ? 2 : 1): 0) 
// Turning direction: 
//   Most significant bit of U = 1: Increasing heading, Ship turns right 
//   Most significant bit of U = 0: Decreasing heading, Ship turns left 
// Autopilot course in degrees: 
//   The two higher bits of  V * 90 + XY / 2 
// Z & 0x2 = 0 : Autopilot in Standby-Mode 
// Z & 0x2 = 2 : Autopilot in Auto-Mode 
// Z & 0x4 = 4 : Autopilot in Vane Mode (WindTrim), requires regular "10" datagrams 
// Z & 0x8 = 8 : Autopilot in Track Mode
// M: Alarms + audible beeps 
//   M & 0x04 = 4 : Off course 
//   M & 0x08 = 8 : Wind Shift
// Rudder position: RR degrees (positive values steer right, 
//   negative values steer left. Example: 0xFE = 2° left) 
// SS & 0x01 : when set, turns off heading display on 600R control. 
// SS & 0x02 : always on with 400G 
// SS & 0x08 : displays “NO DATA” on 600R 
// SS & 0x10 : displays “LARGE XTE” on 600R 
// SS & 0x80 : Displays “Auto Rel” on 600R 
// TT : Always 0x08 on 400G computer, always 0x05 on 150(G) computer 
class SeaTalkMessageCompassHeadingAutopilotCourseRudderPosition : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageCompassHeadingAutopilotCourseRudderPosition(const uint8_t *message) : BaseSeaTalkMessage(message, this->messageLength()) {}
    int messageLength() { return 9; }
    int compassHeading() {
        int heading = ((_message[1] & 0x30) >> 4) * 90;
        heading += (_message[2] & 0x3F) * 2;
        heading += (_message[1] & 0x80) >> 7;
        return heading;
    }
    bool isTurningRight() { return (_message[1] & 0x80) == 0x80; }
    int autopilotCourse() {
        return ((_message[2] & 0xC0) >> 6) * 90 + (_message[3] / 2);
    }
    bool isAutoMode() {
        return (_message[4] & 0x2) == 0x2;
    }
    bool isVaneMode() {
        return (_message[4] & 0x4) == 0x4;
    }
    bool isTrackMode() {
        return (_message[4] & 0x8) == 0x8;
    }
};

// 99  00  XX        Compass variation sent by ST40 compass instrument
// or ST1000, ST2000, ST4000+, E-80 every 10 seconds
// but only if the variation is set on the instrument
// Positive XX values: Variation West, Negative XX values: Variation East
// Examples (XX => variation): 00 => 0, 01 => -1 west, 02 => -2 west ...
//                             FF => +1 east, FE => +2 east ...
class SeaTalkMessageMagneticVariation : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageMagneticVariation(int variation);
    int messageLength() { return 3; }
    int varation() { return (int)_message[2]; }
};

 // 9C  U1  VW  RR    Compass heading and Rudder position (see also command 84)
 // Compass heading in degrees:
 //   The two lower  bits of  U * 90 +
 //   the six lower  bits of VW *  2 +
 //   number of bits set in the two higher bits of U =
 //   (U & 0x3)* 90 + (VW & 0x3F)* 2 + (U & 0xC ? (U & 0xC == 0xC ? 2 : 1): 0)
 // Turning direction:
 //   Most significant bit of U = 1: Increasing heading, Ship turns right
 //   Most significant bit of U = 0: Decreasing heading, Ship turns left
 // Rudder position: RR degrees (positive values steer right,
 //   negative values steer left. Example: 0xFE = 2° left)
class SeaTalkMessageCompassHeadingAndRudderPosition : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageCompassHeadingAndRudderPosition(const uint8_t *message) : BaseSeaTalkMessage(message, this->messageLength()) {}
    SeaTalkMessageCompassHeadingAndRudderPosition(int compassHeading, bool isTurningRight, int rudderPosition);
    int messageLength() { return 4; }
    int compassHeading() {
        int heading = ((_message[1] & 0x30) >> 4) * 90;
        heading += (_message[2] & 0x3F) * 2;
        heading += (_message[1] & 0x80) >> 7;
        return heading;
    }
    bool isTurningRight() { return (_message[1] & 0x40) == 0x80; }
    int rudderPosition() { return (int)_message[3]; }
};

class SeaTalkMessageDeviceQuery : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageDeviceQuery();
    int messageLength() { return 5; }
};

BaseSeaTalkMessage *newSeaTalkMessage(const uint8_t *message, int messageLength);

void printSeaTalkMessage(uint8_t *message, int messageLength);

#endif
