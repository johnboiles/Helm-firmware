#ifndef NMEAShared_h
#define NMEAShared_h

#include "Types.h"

int calculateChecksum(char *message, int length);

double degreesFromCoordinateString(const char *string, char direction);

void splitMessageIntoFragments(const char *message, int messageLength, char **fragments, int *fragmentCount);

Time timeFromString(const char *timeString);

#endif
