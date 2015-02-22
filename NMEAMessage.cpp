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
    sprintf(&(message[messageLength]), "%02X", checksum);
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
    }
}


NMEAMessageRMB::NMEAMessageRMB(const char *message) : BaseNMEAMessage() {
    char *fragments[15];
    int fragmentCount = 0;
    splitMessageIntoFragments(message, strlen(message), fragments, &fragmentCount);

    _status = statusFromFragment(fragments[1]);
    _xte = atof(fragments[2]);
    _directionToSteer = lateralityFromFragment(fragments[3]);
    COPY_STRING(_toWaypointID, fragments[4]);
    COPY_STRING(_fromWaypointID, fragments[5]);
    _destinationLatitude = degreesFromCoordinateString(fragments[6], fragments[7][0]);
    _destinationLongitude = degreesFromCoordinateString(fragments[8], fragments[9][0]);
    _rangeToDestiation = atof(fragments[10]);
    _bearingToDestination = headingFromFragments(fragments[11], "T");
    _destinationClosingVelocity = atof(fragments[12]);
    _isArrived = fragments[13][0] == 'A' ? true : false;

    for (int i = 0; i < fragmentCount; i++) {
        free(fragments[i]);
    }
}


NMEAMessageRMC::NMEAMessageRMC(const char *message) : BaseNMEAMessage() {
    char *fragments[12];
    int fragmentCount = 0;
    splitMessageIntoFragments(message, strlen(message), fragments, &fragmentCount);

    _time = timeFromString(fragments[1]);
    _status = statusFromFragment(fragments[2]);
    _latitude = degreesFromCoordinateString(fragments[3], fragments[4][0]);
    _longitude = degreesFromCoordinateString(fragments[5], fragments[6][0]);
    _speedOverGround = atof(fragments[7]);
    _trackMadeGood = headingFromFragments(fragments[8], "T");
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


NMEAMessageAPB::NMEAMessageAPB(const char *message) : BaseNMEAMessage() {
    char *fragments[15];
    int fragmentCount = 0;
    splitMessageIntoFragments(message, strlen(message), fragments, &fragmentCount);

    _isUnreliableFix = fragments[1][0] == 'V' ? true : false;
    _isCycleLockWarning = fragments[2][0] == 'V' ? true : false;
    _xte = atof(fragments[3]);
    _directionToSteer = lateralityFromFragment(fragments[4]);
    // TODO: Do XTE units ever change? If so, implement units for XTE
    _isArrived = fragments[6][0] == 'A' ? true : false;
    _isPerpendicularPassed = fragments[7][0] == 'A' ? true : false;
    _bearingOriginToDestination = headingFromFragments(fragments[8], fragments[9]);
    COPY_STRING(_destinationWaypointID, fragments[10]);
    _bearingPresentToDestination = headingFromFragments(fragments[11], fragments[12]);
    _headingToSteerToWaypoint = headingFromFragments(fragments[13], fragments[14]);

    for (int i = 0; i < fragmentCount; i++) {
        free(fragments[i]);
    }
}


NMEAMessageSEA::NMEAMessageSEA(const char *message) : BaseNMEAMessage() {
    char *fragments[3];
    int fragmentCount = 0;
    splitMessageIntoFragments(message, strlen(message), fragments, &fragmentCount);

    // Convert ascii hex to binary
    _seaTalkMessageLength = strlen(fragments[1]) / 2;
    for (int i = 0; i < _seaTalkMessageLength; i++) {
        _seaTalkMessage[i] = (asciiHexToBinary(fragments[1][2 * i]) << 4) + asciiHexToBinary(fragments[1][2 * i + 1]);
    }
}


NMEAMessageSEA::NMEAMessageSEA(const uint8_t *seaTalkMessage, uint8_t seaTalkMessageLength) {
    memcpy(&_seaTalkMessage, seaTalkMessage, seaTalkMessageLength);
    _seaTalkMessageLength = seaTalkMessageLength;
    
    sprintf(_message, "$STSEA,");
    uint8_t headerLength = 7;

    // Convert binary to ascii hex
    for (int i = 0; i < seaTalkMessageLength; i++) {
        sprintf(&(_message[headerLength + 2 * i]), "%02X", seaTalkMessage[i]);
    }
    closeMessage(_message);
}
