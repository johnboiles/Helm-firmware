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
    SeaTalkMessageTypeNavigationToWaypoint = 0x85,
    SeaTalkMessageTypeMagneticVariation = 0x99,
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
protected:
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

// 82  05  XX  xx YY yy ZZ zz   Target waypoint name
// XX+xx = YY+yy = ZZ+zz = FF (allows error detection)
// Takes the last 4 chars of name, assumes upper case only
// Char= ASCII-Char - 0x30
// XX&0x3F: char1
// (YY&0xF)*4+(XX&0xC0)/64: char2
// (ZZ&0x3)*16+(YY&0xF0)/16: char3
// (ZZ&0xFC)/4: char4
class SeaTalkMessageTargetWaypointName : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageTargetWaypointName(const char *name);
    SeaTalkMessageTargetWaypointName(const uint8_t *message);
    int messageLength() { return 8; }
    char *name() { return _name; }
private:
    char _name[5];
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

// 85  X6  XX  VU ZW ZZ YF 00 yf   Navigation to waypoint information
// Cross Track Error: XXX/100 nautical miles
//   Example: X-track error 2.61nm => 261 dec => 0x105 => X6XX=5_10
// Bearing to destination: (U & 0x3) * 90° + WV / 2°
//   Example: GPS course 230°=180+50=2*90 + 0x64/2 => VUZW=42_6
//   U&8: U&8 = 8 -> Bearing is true, U&8 = 0 -> Bearing is magnetic
// Distance to destination: Distance 0-9.99nm: ZZZ/100nm, Y & 1 = 1
//   Distance >=10.0nm: ZZZ/10 nm, Y & 1 = 0
// Direction to steer: if Y & 4 = 4 Steer right to correct error
//   if Y & 4 = 0 Steer left  to correct error
//   Example: Distance = 5.13nm, steer left: 5.13*100 = 513 = 0x201 => ZW ZZ YF=1_ 20 1_
//   Distance = 51.3nm, steer left: 51.3*10  = 513 = 0x201 => ZW ZZ YF=1_ 20 0_
// Track control mode:
//   F= 0x1: Display x-track error and Autopilot course
//   F= 0x3: Enter Track Control Mode, i.e. lock on to GPS.
//     Display x-track error, autopilot course and bearing
//     to destination
//   F= 0x5: Display x-track error, distance to waypoint,
//     autopilot course and bearing to destination
//     normal--> F= 0x7: Enter Track Control Mode, i.e. lock on to GPS.
//     Display x-track error, distance to waypoint,
//     autopilot course and bearing to destination
//   F= 0xF: As 0x7 but with x-track error alarm
//   F= 2, 4, 6, 8 ... causes data errors
// In case of a waypoint change, sentence 85, indicating the new bearing and distance,
// should be transmitted prior to sentence 82 (which indicates the waypoint change).
class SeaTalkMessageNavigationToWaypoint : public BaseSeaTalkMessage
{
public:
    SeaTalkMessageNavigationToWaypoint(float xte, Heading bearingToDestination, float distanceToDestination, Laterality directionToSteer, int trackControlMode);
    int messageLength() { return 9; }
    float xte() { return _xte; }
    Heading bearingToDestination() { return _bearingToDestination; }
    float distanceToDestination() { return _distanceToDestination; }
    Laterality directionToSteer() { return _directionToSteer; }
    int trackControlMode() { return _trackControlMode; }
private:
    float _xte;
    Heading _bearingToDestination;
    float _distanceToDestination;
    Laterality _directionToSteer;
    // TODO: Create an enum for this?
    int _trackControlMode;
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
    SeaTalkMessageMagneticVariation(const uint8_t *message) : BaseSeaTalkMessage(message, this->messageLength()) {}
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
