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
        uint8_t getRssi(void);
        void setRssi(uint8_t value);
        uint8_t getSnr(void);
        void setSnr(uint8_t value);
        uint32_t getLastContactMillis(void);
        void setLastContactMillis(uint32_t value);
        bool hasPos(void);
    private:
        uint32_t _id;
        bool _hasPos;
        double _lat;
        double _lon;
        double _alt;
        uint8_t _rssi;
        uint8_t _snr;
        uint32_t _lastContactMillis;

};

#endif