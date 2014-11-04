#ifndef NMEAShared_h
#define NMEAShared_h


int calculateChecksum(char *message, int length);

double degreesFromCoordinateString(const char *string, char direction);

#endif
