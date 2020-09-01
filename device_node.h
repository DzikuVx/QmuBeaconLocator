#pragma once

#ifndef DEVICE_NODE_H
#define DEVICE_NODE_H

#include <Arduino.h>
#include <QmuTactile.h>
#include "oled_display.h"
#include "radio_node.h"
#include "config_node.h"

extern QmuTactile buttonMain;
extern OledDisplay oledDisplay;
extern QspConfiguration_t qsp;
extern RadioNode radioNode;
extern ConfigNode configNode;
extern Beacons beacons;

enum deviceModes {
    DEVICE_MODE_LOCATOR = 0,
    DEVICE_MODE_BEACON,
    DEVICE_MODE_LAST
};

class DeviceNode {
    public:
        DeviceNode(uint16_t taskTxMs);
        void processInputs(void);
        void execute(void);
    private:
        uint16_t taskTxMs = 200;
        uint8_t currentDeviceMode = DEVICE_MODE_LOCATOR;
        uint8_t previousDeviceMode = DEVICE_MODE_LOCATOR;
        uint32_t nextLoRaTxTaskTs = 0;
};

#endif

