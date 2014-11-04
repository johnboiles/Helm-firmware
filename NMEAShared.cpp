#include "NMEAShared.h"
#include <Arduino.h>
#include <stdio.h>

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
