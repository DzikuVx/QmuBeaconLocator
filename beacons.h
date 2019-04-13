#pragma once

#ifndef BEACONS_H
#define BEACONS_H

#include "Arduino.h"
#include "variables.h"
#include "beacon.h"

#define BEACONS_MAX_COUNT 8

class Beacons {
    public:
        Beacons(void);
        void updatePosition(uint32_t beaconId, double lat, double lon, double alt);
        Beacon *getBeacon(uint32_t id);
        Beacon *getEmpty(void);
        Beacon *get(uint8_t index);
        uint8_t count(void);
    private:
        Beacon _beacons[BEACONS_MAX_COUNT];
};

#endif