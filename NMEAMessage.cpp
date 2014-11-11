#include "NMEAMessage.h"
#include <cstring>
#include "NMEAShared.h"
#include <stdio.h>
#include "Arduino.h"
#include "stdlib.h"


BaseNMEAMessage::BaseNMEAMessage() {
    memset(_message, 0, sizeof(_message));
}


const char *BaseNMEAMessage::message() {
    return _message;
}


NMEAMessageWind::NMEAMessageWind(float windAngle, float windSpeed) : BaseNMEAMessage() {
    sprintf(_message, "$WIMWV,%.1f,R,%.1f,N,A", windAngle, windSpeed);
    int messageLength = strlen(_message);
    int checksum = calculateChecksum(&(_message[1]), messageLength - 1);
    _message[messageLength++] = '*';
    sprintf(&(_message[messageLength]), "%2X", checksum);
    messageLength += 2;
    sprintf(&(_message[messageLength]), "\r\n");
}


NMEAMessageGLL::NMEAMessageGLL(const char *message) : BaseNMEAMessage() {
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


NMEAMessageRMC::NMEAMessageRMC(const char *message) : BaseNMEAMessage() {
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


NMEAMessageDBT::NMEAMessageDBT(float depth) : BaseNMEAMessage() {
    sprintf(_message, "$STDBT,%.1f,f,,M,,F", depth);
    int messageLength = strlen(_message);
    int checksum = calculateChecksum(&(_message[1]), messageLength - 1);
    _message[messageLength++] = '*';
    sprintf(&(_message[messageLength]), "%2X", checksum);
    messageLength += 2;
    sprintf(&(_message[messageLength]), "\r\n");
}
