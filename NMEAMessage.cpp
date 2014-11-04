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
    int messageLength = strlen(message);
    char *fragments[10];
    char checksum[2];
    int fragmentCount = 0;
    int fragmentStartIndex = 0;

    // Split into fragments by commas
    for (int i = 0; i < messageLength; i++) {
        if (message[i] == ',' || message[i] == '*') {
            int fragmentLength = i - fragmentStartIndex;
            char *fragment;
            if (fragmentLength > 0) {
                fragment = (char *)malloc((fragmentLength + 1) * sizeof(char));
                memcpy(fragment, &message[fragmentStartIndex], fragmentLength);
                // Null terminate for easy printing
                fragment[fragmentLength] = 0;
            } else {
                // Fill empty fragments with the empty string
                fragment = (char *)malloc(2 * sizeof(char));
                strcpy(fragment, "");
            }
            fragments[fragmentCount++] = fragment;
            fragmentStartIndex = i + 1;
        }
        if (message[i] == '*') {
            memcpy(checksum, &message[i + 1], 2);
            break;
        }
    }

    _latitude = degreesFromCoordinateString(fragments[1], fragments[2][0]);
    _longitude = degreesFromCoordinateString(fragments[3], fragments[4][0]);
    sscanf(fragments[5], "%2d%2d", &_hour, &_minute);
    // TODO: Can't figure out how to read floats with sscanf
    _second = atof(&fragments[5][4]);

    for (int i = 0; i < fragmentCount; i++) {
      free(fragments[i]);
      // Serial.println(fragments[i]);
    }
}
