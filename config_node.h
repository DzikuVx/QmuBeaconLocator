#pragma once

#include "Arduino.h"
#include "radio_node.h"
#include <EEPROM.h>

#ifndef PLATFORM_NODE_H
#define PLATFORM_NODE_H
 
#define EEPROM_SIZE 128

enum platformConfigMemoryLayout { 
    EEPROM_ADDRESS_BEACONID_KEY_SEEDED = 0x00, 
    EEPROM_ADDRESS_BEACONID_0, 
    EEPROM_ADDRESS_BEACONID_1, 
    EEPROM_ADDRESS_BEACONID_2, 
    EEPROM_ADDRESS_BEACONID_3,
    EEPROM_ADDRESS_DEVICE_MODE,
    EEPROM_ADDRESS_ACTION_ENABLED,
    CONFIG_NODE_LAST_BYTE 
}; 

class ConfigNode {

    public:
        ConfigNode(void);
        void begin(void);
        void seed(void);
        long loadBeaconId(void);
        void saveBeaconId(long key);
        uint8_t load(int address);
        void save(int address, uint8_t value);
        void commit(void);
        unsigned long beaconId;
};

#endif
