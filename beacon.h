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
        double getLat(void);
        double getLon(void);
        double getAlt(void);
        int getCourse(void);
        uint8_t getActionRaw(void);
        uint8_t getFlagsRaw(void);
        void setLat(double lat);
        void setLon(double lon);
        void setAlt(double alt);
        void setCourse(int course);
        void setAction(uint8_t action);
        void setFlags(uint8_t flags);
        uint8_t getRssi(void);
        void setRssi(uint8_t value);
        uint8_t getSnr(void);
        void setSnr(uint8_t value);
        uint32_t getLastContactMillis(void);
        void setLastContactMillis(uint32_t value);
        bool hasPos(void);
        void setSats(uint8_t value);
        void setHdop(long value);
        uint8_t getSats(void);
        long getHdop(void);
        void setSpeed(double value);
        double getSpeed(void);
    private:
        uint32_t _id;
        bool _hasPos;
        double _lat;
        double _lon;
        double _alt;
        int _course;  
        long _hdop;
        long _speed;
        uint8_t _sats;
        uint8_t _rssi;
        uint8_t _snr;
        uint8_t _flags;
        uint8_t _action;
        uint32_t _lastContactMillis;

};

#endif