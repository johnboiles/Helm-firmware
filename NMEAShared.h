#ifndef NMEAShared_h
#define NMEAShared_h

#include "types.h"
#include <cstring>
#include "inttypes.h"


int calculateChecksum(char *message, size_t length);

double degreesFromCoordinateString(const char *string, char direction);

void splitMessageIntoFragments(const char *message, size_t messageLength, char **fragments, int *fragmentCount);

Time timeFromString(const char *timeString);

Heading headingFromFragments(const char *degrees, const char *trueOrMagnetic);

Laterality lateralityFromFragment(const char *fragment);

Status statusFromFragment(const char *fragment);

uint8_t asciiHexToBinary(char asciiHex);

#endif
