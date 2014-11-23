#ifndef NMEAMessage_h
#define NMEAMessage_h

#include "types.h"

typedef enum {
    ACTIVE = 'A',
    VOID = 'V'
} STATUS;

typedef enum {
    LEFT = 'L',
    RIGHT = 'R',
    UNKNOWN = 0
} LEFT_OR_RIGHT;

class BaseNMEAMessage
{
public:
    BaseNMEAMessage();
    const char *message();
    // TODO: This should be private but I forget how friend classes work in C++
    char _message[100];
};

class NMEAMessageWind : public BaseNMEAMessage
{
public:
    NMEAMessageWind(float windAngle, float windSpeed);
};

class NMEAMessageGLL : public BaseNMEAMessage
{
public:
    NMEAMessageGLL(const char *message);
    double latitude() { return _latitude; }
    double longitude() { return _longitude; }
    Time time() { return _time; }
private:
    double _latitude;
    double _longitude;
    Time _time;
};


class NMEAMessageRMB : public BaseNMEAMessage
{
public:
    NMEAMessageRMB(const char *message);
    STATUS status() { return _status; }
    float xte() { return _xte; }
    LEFT_OR_RIGHT directionToSteer() { return _directionToSteer; }
    char *toWaypointID() { return _toWaypointID; }
    char *fromWaypointID() { return _fromWaypointID; }
    double destinationLatitude() { return _destinationLatitude; }
    double destinationLongitude() { return _destinationLongitude; }
    float rangeToDestiation() { return _rangeToDestiation; }
    float bearingToDestination() { return _bearingToDestination; }
    float destinationClosingVelocity() { return _destinationClosingVelocity; }
    bool isArrived() { return _isArrived; }
private:
    STATUS _status;
    float _xte;
    LEFT_OR_RIGHT _directionToSteer;
    char _toWaypointID[20];
    char _fromWaypointID[20];
    double _destinationLatitude;
    double _destinationLongitude;
    float _rangeToDestiation;
    float _bearingToDestination;
    float _destinationClosingVelocity;
    bool _isArrived;
};

class NMEAMessageRMC : public BaseNMEAMessage
{
public:
    NMEAMessageRMC(const char *message);
    Time time() { return _time; }
    // TODO: Status
    double latitude() { return _latitude; }
    double longitude() { return _longitude; }
    double speedOverGround() { return _speedOverGround; }
    double trackMadeGood() { return _trackMadeGood; }
    Date date() { return _date; }
    //! West is negative
    double magneticVariation() { return _magneticVariation; }
private:
    Time _time;
    double _latitude;
    double _longitude;
    double _speedOverGround;
    double _trackMadeGood;
    Date _date;
    double _magneticVariation;
};


//! Depth below transducer
class NMEAMessageDBT : public BaseNMEAMessage
{
public:
    //! Depth in feet
    NMEAMessageDBT(float depth);
};


//! Water speed and heading
class NMEAMessageVHW : public BaseNMEAMessage
{
public:
    //! Water speed in Knots
    // TODO: There is all sorts of other data in this message. Leaving it out for now since I don't need it.
    NMEAMessageVHW(float knots);
};


//! Magnetic Heading
class NMEAMessageHDM : public BaseNMEAMessage
{
public:
    //! Magnetic heading in degrees
    NMEAMessageHDM(float degrees);
};


#endif
