#pragma once

#ifndef DEVICE_NODE_H
#define DEVICE_NODE_H

#include <Arduino.h>
#include <QmuTactile.h>
#include "radio_node.h"
#include "config_node.h"
#include <TinyGps++.h>

extern QmuTactile buttonMain;
extern QspConfiguration_t qsp;
extern RadioNode radioNode;
extern ConfigNode configNode;
extern Beacons beacons;
extern TinyGPSPlus gps;

enum deviceModes {
    DEVICE_MODE_LOCATOR = 0,
    DEVICE_MODE_BEACON,
    DEVICE_MODE_LOOK_AT_ME,
    DEVICE_MODE_LAST,
};

class DeviceNode {
    public:
        DeviceNode(uint16_t taskTxMs);
        void begin(void);
        void processInputs(void);
        void execute(void);
        bool isActionEnabled(void);
        uint8_t getDeviceMode(void);
    private:
        uint16_t taskTxMs = 200;
        uint8_t currentDeviceMode = DEVICE_MODE_LOCATOR;
        uint8_t previousDeviceMode = DEVICE_MODE_LOCATOR;
        bool _actionEnabled = false;
        bool _previousActionEnabled = false;
        uint32_t nextLoRaTxTaskTs = 0;
        bool _forcedInit = false;
};

#endif

