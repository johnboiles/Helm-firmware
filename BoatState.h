//
//  BoatState.h
//  Helm
//
//  Created by John Boiles on 12/5/14.
//  Copyright (c) 2014 johnboiles. All rights reserved.
//

#ifndef BoatState_h
#define BoatState_h

#include "types.h"

class BoatState
{
public:
    BoatState() {
        magneticVariation = 0;
        windSpeed = 0;
        windAngle = 0;
    }
    float magneticVariation;
    float windSpeed;
    float windAngle;
    Heading headingToMagnetic(Heading heading) {
        if (!heading.isMagnetic) {
            heading.degrees -= this->magneticVariation;
            heading.isMagnetic = true;
        }
        return heading;
    }
    Heading headingToTrue(Heading heading) {
        if (heading.isMagnetic) {
            heading.degrees += this->magneticVariation;
            heading.isMagnetic = false;
        }
        return heading;
    }
};

#endif
