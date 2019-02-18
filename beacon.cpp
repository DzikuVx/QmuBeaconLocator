#include "beacon.h"

Beacon::Beacon(void) {

};

uint32_t Beacon::getId(void) {
    return _id;
};

void Beacon::setId(uint32_t id) {
    _id = id;
};

void Beacon::setPos(double lat, double lon, double alt) {
    _lat = lat;
    _lon = lon;
    _alt = alt;
};

double Beacon::getLat(void) {
    return _lat;
}

double Beacon::getLon(void) {
    return _lon;
}

double Beacon::getAlt(void) {
    return _alt;
}

void Beacon::setLat(double lat) {
    _lat = lat;
}

void Beacon::setLon(double lon) {
    _lon = lon;
}

void Beacon::setAlt(double alt) {
    _alt = alt;
}