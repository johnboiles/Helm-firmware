#include "NMEAMessage.h"
#include <cstring>
#include "NMEAShared.h"
#include <stdio.h>
#include <Arduino.h>

const char *BaseNMEAMessage::message() {
    return _message;
}


NMEAMessageWind::NMEAMessageWind(float windAngle, float windSpeed) {
    memset(_message, 0, sizeof(_message));
    sprintf(_message, "$WIMWV,%.1f,R,%.1f,N,A", windAngle, windSpeed);
    int messageLength = strlen(_message);
    int checksum = calculateChecksum(&(_message[1]), messageLength - 1);
    _message[messageLength++] = '*';
    sprintf(&(_message[messageLength]), "%2X", checksum);
    messageLength += 2;
    sprintf(&(_message[messageLength]), "\r\n");
}


NMEAMessageGLL::NMEAMessageGLL(const char *message) {
    char *fragments[10];
    int fragmentCount = 0;
    splitMessageIntoFragments(message, strlen(message), fragments, &fragmentCount);

    _latitude = degreesFromCoordinateString(fragments[1], fragments[2][0]);
    _longitude = degreesFromCoordinateString(fragments[3], fragments[4][0]);
    _time = timeFromString(fragments[5]);
    for (int i = 0; i < fragmentCount; i++) {
      free(fragments[i]);
      // Serial.println(fragments[i]);
    }
}

//                                                          12
//         1         2 3       4 5        6  7   8   9    10 11|  13
//         |         | |       | |        |  |   |   |    |  | |   |
//  $--RMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,xxxx,x.x,a,m,*hh<CR><LF>
// Field Number:

// 1 UTC Time

// 2 Status, V=Navigation receiver warning A=Valid

// 3 Latitude

// 4 N or S

// 5 Longitude

// 6 E or W

// 7Speed over ground, knots
 
// 8 Track made good, degrees true

// 9 Date, ddmmyy

// 10 Magnetic Variation, degrees

// 11 E or W

// 12 FAA mode indicator (NMEA 2.3 and later)

// Checksum

// A status of V means the GPS has a valid fix that is below an internal quality threshold, e.g. because the dilution of precision is too high or an elevation mask test failed.

NMEAMessageRMC::NMEAMessageRMC(const char *message) {
    char *fragments[12];
    int fragmentCount = 0;
    splitMessageIntoFragments(message, strlen(message), fragments, &fragmentCount);

    _time = timeFromString(fragments[1]);
    // TODO: Parse status (fragments[2])
    _latitude = degreesFromCoordinateString(fragments[3], fragments[4][0]);
    _longitude = degreesFromCoordinateString(fragments[5], fragments[6][0]);
    _speedOverGround = atof(fragments[7]);
    _trackMadeGood = atof(fragments[8]);
    sscanf(fragments[9], "%2d%2d%2d", &(_date.day), &(_date.month), &(_date.year));
    _magneticVariation = atof(fragments[10]);
    if (fragments[11][0] == 'W') {
        _magneticVariation = _magneticVariation * -1;
    }

    for (int i = 0; i < fragmentCount; i++) {
      free(fragments[i]);
    }
}
