#include "config_node.h"
#include "utils.h"
#include "EEPROM.h"

ConfigNode::ConfigNode(void) {
    
}

void ConfigNode::begin(void) {
    while (!EEPROM.begin(EEPROM_SIZE)) {
        true;
    }
}

/**
 * Return true if new bind key was generated
 */
void ConfigNode::seed(void) {
    uint8_t val;

    val = EEPROM.read(EEPROM_ADDRESS_BEACONID_KEY_SEEDED);
    if (val != 0xf1) {
        EEPROM.write(EEPROM_ADDRESS_BEACONID_0, random(1, 255)); //Yes, from 1 to 254
        EEPROM.write(EEPROM_ADDRESS_BEACONID_1, random(1, 255)); //Yes, from 1 to 254
        EEPROM.write(EEPROM_ADDRESS_BEACONID_2, random(1, 255)); //Yes, from 1 to 254
        EEPROM.write(EEPROM_ADDRESS_BEACONID_3, random(1, 255)); //Yes, from 1 to 254
        EEPROM.write(EEPROM_ADDRESS_BEACONID_KEY_SEEDED, 0xf1);
        EEPROM.commit();
    }
}

long ConfigNode::loadBeaconId(void) {
    long tmp;

    tmp = EEPROM.read(EEPROM_ADDRESS_BEACONID_0);
    tmp += ((long)EEPROM.read(EEPROM_ADDRESS_BEACONID_1)) << 8;
    tmp += ((long)EEPROM.read(EEPROM_ADDRESS_BEACONID_2)) << 16;
    tmp += ((long)EEPROM.read(EEPROM_ADDRESS_BEACONID_3)) << 24;

    beaconId = tmp;
    return tmp;
}

void ConfigNode::saveBeaconId(long key) {

    uint8_t tmp[4];

    int32ToBuf(tmp, 0, key);

    EEPROM.write(EEPROM_ADDRESS_BEACONID_0, tmp[0]);
    EEPROM.write(EEPROM_ADDRESS_BEACONID_1, tmp[1]);
    EEPROM.write(EEPROM_ADDRESS_BEACONID_2, tmp[2]);
    EEPROM.write(EEPROM_ADDRESS_BEACONID_3, tmp[3]);
    EEPROM.write(EEPROM_ADDRESS_BEACONID_KEY_SEEDED, 0xf1);
    EEPROM.commit();
}

uint8_t ConfigNode::load(int address) {
    return EEPROM.read(address);
}

void ConfigNode::save(int address, uint8_t value) {
    EEPROM.write(address, value);
}

void ConfigNode::commit(void) {
    EEPROM.commit();
}