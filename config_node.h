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
    CONFIG_NODE_LAST_BYTE 
}; 

class ConfigNode {

    public:
        ConfigNode(void);
        void begin(void);
        void seed(void);
        long loadBeaconId(void);
        void saveBeaconId(long key);
        long beaconId;
};

#endif
