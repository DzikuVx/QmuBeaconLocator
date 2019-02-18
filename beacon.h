#pragma once

#ifndef BEACON_H
#define BEACON_H

#include "Arduino.h"
#include "variables.h"

class Beacon {
    public:
        Beacon(void);
        uint32_t getId(void);
        void setId(uint32_t id);
        void setPos(double lat, double lon, double alt);
        double getLat(void);
        double getLon(void);
        double getAlt(void);
        void setLat(double lat);
        void setLon(double lon);
        void setAlt(double alt);
    private:
        uint32_t _id;
        double _lat;
        double _lon;
        double _alt;

};

#endif