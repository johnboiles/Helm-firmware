#include "NMEAMessage.h"
#include <cstring>
#include "NMEAShared.h"
#include <stdio.h>
#include "Arduino.h"
#include "stdlib.h"


void closeMessage(char *message) {
    size_t messageLength = strlen(message);
    int checksum = calculateChecksum(&(message[1]), messageLength - 1);
    message[messageLength++] = '*';
    sprintf(&(message[messageLength]), "%2X", checksum);
    messageLength += 2;
    sprintf(&(message[messageLength]), "\r\n");
}


BaseNMEAMessage::BaseNMEAMessage() {
    memset(_message, 0, sizeof(_message));
}


const char *BaseNMEAMessage::message() {
    return _message;
}


NMEAMessageWind::NMEAMessageWind(float windAngle, float windSpeed) : BaseNMEAMessage() {
    sprintf(_message, "$WIMWV,%.1f,R,%.1f,N,A", windAngle, windSpeed);
    closeMessage(_message);
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


NMEAMessageRMB::NMEAMessageRMB(const char *message) : BaseNMEAMessage() {
    char *fragments[15];
    int fragmentCount = 0;
    splitMessageIntoFragments(message, strlen(message), fragments, &fragmentCount);

    if (fragments[1][0] == VOID) {
        _status = VOID;
    } else {
        _status = ACTIVE;
    }
    _xte = atof(fragments[2]);
    if (fragments[3][0] == 'L') {
        _directionToSteer = LEFT;
    } else if (fragments[3][0] == 'R') {
        _directionToSteer = RIGHT;
    } else {
        _directionToSteer = UNKNOWN;
    }
    strncpy(_toWaypointID, fragments[4], min(sizeof(_toWaypointID) - 1, strlen(fragments[4])) + 1);
    strncpy(_fromWaypointID, fragments[5], min(sizeof(_fromWaypointID) - 1, strlen(fragments[5])) + 1);
    _destinationLatitude = degreesFromCoordinateString(fragments[6], fragments[7][0]);
    _destinationLongitude = degreesFromCoordinateString(fragments[8], fragments[9][0]);
    _rangeToDestiation = atof(fragments[10]);
    _bearingToDestination = atof(fragments[11]);
    _destinationClosingVelocity = atof(fragments[12]);
    _isArrived = fragments[13][0] == 'A' ? true : false;

    // TODO: Verify checksum

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
    closeMessage(_message);
}


NMEAMessageVHW::NMEAMessageVHW(float knots) : BaseNMEAMessage() {
    sprintf(_message, "$STVHW,,T,,M,%.1f,N,,K", knots);
    closeMessage(_message);
}


NMEAMessageHDM::NMEAMessageHDM(float heading) : BaseNMEAMessage() {
    sprintf(_message, "$STHDM,%.1f,M", heading);
    closeMessage(_message);
}
