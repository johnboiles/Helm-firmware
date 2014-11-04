#ifndef NMEAMessage_h
#define NMEAMessage_h


class BaseNMEAMessage
{
public:
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
    int hour() { return _hour; }
    int minute() { return _minute; }
    double second() { return _second; }
private:
    double _latitude;
    double _longitude;
    int _hour;
    int _minute;
    double _second;
};

#endif
