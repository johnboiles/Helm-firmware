#include "NMEAShared.h"
#include <Arduino.h>
#include <stdio.h>
#include "types.h"

int calculateChecksum(char *message, int length) {
  int c = 0;
    for (int i = 0; i < length; i++) {
        c ^= message[i];
    }
    return c;
}

//! NMEA degrees are of the format 3751.98291 where the first 2-3 characters are the degrees,
//  Then the rest is minutes
double degreesFromCoordinateString(const char *string, char direction) {
    // Edge case, don't process strings that are too short
    int length = strlen(string);
    if (length < 3) {
        return 0.0;
    }
    int decimalIndex = strchr(string, '.') - string;
    double minutes = atof(&(string[decimalIndex - 2]));
    int i = decimalIndex - 3;
    int degrees = 0;
    int multiplier = 1;
    while (i >= 0) {
      degrees += (string[i] - '0') * multiplier;
      i--;
      multiplier *= 10;
    }
    double coordinate = (double)degrees + (minutes/60.0);
    if (direction == 'S' || direction == 'W') {
        coordinate = coordinate * -1;
    }
    return coordinate;
}

void splitMessageIntoFragments(const char *message, int messageLength, char **fragments, int *fragmentCount) {
    *fragmentCount = 0;
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
            fragments[(*fragmentCount)++] = fragment;
            fragmentStartIndex = i + 1;
        }
        if (message[i] == '*') {
            break;
        }
    }
}

Time timeFromString(const char *timeString) {
    Time time;
    sscanf(timeString, "%2d%2d", &(time.hour), &(time.minute));
    // TODO: Can't figure out how to read floats with sscanf
    time.second = atof(&timeString[4]);
    return time;
}
