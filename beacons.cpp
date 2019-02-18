#include "beacons.h"

Beacons::Beacons(void) {

}

void Beacons::updatePosition(uint32_t beaconId, double lat, double lon, double alt) {
    Beacon *beacon = getBeacon(beaconId);

    beacon->setPos(lat, lon, alt);
}

Beacon *Beacons::getEmpty(void) {
    for (uint8_t i = 0; i < BEACONS_MAX_COUNT; i++) {
        if (!_beacons[i].getId()) {
            return &_beacons[i];
        }
    }
    return NULL;
}

Beacon *Beacons::getBeacon(uint32_t id) {
    for (uint8_t i = 0; i < BEACONS_MAX_COUNT; i++) {
        if (_beacons[i].getId() == id) {
            return &_beacons[i];
        }
    }
    Beacon *b = getEmpty();
    b->setId(id);

    return b;
}
