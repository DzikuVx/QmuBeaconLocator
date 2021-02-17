#include "beacon.h"

Beacon::Beacon(void){

};

uint32_t Beacon::getId(void)
{
    return _id;
};

void Beacon::setId(uint32_t id)
{
    _id = id;
};

double Beacon::getLat(void)
{
    return _lat;
}

double Beacon::getLon(void)
{
    return _lon;
}

double Beacon::getAlt(void)
{
    return _alt;
}

void Beacon::setLat(double lat)
{
    _lat = lat;
    _hasPos = true;
}

void Beacon::setLon(double lon)
{
    _lon = lon;
    _hasPos = true;
}

void Beacon::setAlt(double alt)
{
    _alt = alt;
}

uint8_t Beacon::getRssi(void)
{
    return _rssi;
};

void Beacon::setRssi(uint8_t value)
{
    _rssi = value;
};

uint8_t Beacon::getSnr(void)
{
    return _snr;
};

void Beacon::setSnr(uint8_t value)
{
    _snr = value;
};

uint32_t Beacon::getLastContactMillis(void)
{
    return _lastContactMillis;
};

void Beacon::setLastContactMillis(uint32_t value)
{
    _lastContactMillis = value;
}

bool Beacon::hasPos(void)
{
    return _hasPos;
};

void Beacon::setCourse(int course)
{
    _course = course;
}

int Beacon::getCourse(void)
{
    return _course;
}

void Beacon::setAction(uint8_t action)
{
    _action = action;
}

void Beacon::setFlags(uint8_t flags)
{
    _flags = flags;
}

uint8_t Beacon::getActionRaw(void)
{
    return _action;
}

uint8_t Beacon::getFlagsRaw()
{
    return _flags;
}

void Beacon::setSats(uint8_t value)
{
    _sats = value;
}

void Beacon::setHdop(long value)
{
    _hdop = value;
}

uint8_t Beacon::getSats(void)
{
    return _sats;
}

long Beacon::getHdop(void)
{
    return _hdop;
}

void Beacon::setSpeed(double value)
{
    _speed = value;
}

double Beacon::getSpeed(void)
{
    return _speed;
}