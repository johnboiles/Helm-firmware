#ifndef Types_h
#define Types_h

typedef struct {
	int hour;
	int minute;
	float second;
} Time;

typedef struct {
    int day;
    int month;
    int year;
} Date;

typedef struct {
    float degrees;
    bool isMagnetic;
} Heading;

typedef enum {
    StatusActive = 'A',
    StatusVoid = 'V',
    StatusUnknown = 0
} Status;

typedef enum {
    LateralityLeft = 'L',
    LateralityRight = 'R',
    LateralityUnknown = 0
} Laterality;

#endif