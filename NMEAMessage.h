#ifndef NMEAMessage_h
#define NMEAMessage_h

#include "types.h"

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

class NMEAMessageRMC : public BaseNMEAMessage
{
public:
    NMEAMessageRMC(const char *message);
    // TODO: Maybe create a time struct
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

#endif
