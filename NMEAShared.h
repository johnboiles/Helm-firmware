#ifndef NMEAShared_h
#define NMEAShared_h

#include "types.h"
#include <cstring>


int calculateChecksum(char *message, size_t length);

double degreesFromCoordinateString(const char *string, char direction);

void splitMessageIntoFragments(const char *message, size_t messageLength, char **fragments, int *fragmentCount);

Time timeFromString(const char *timeString);

#endif
