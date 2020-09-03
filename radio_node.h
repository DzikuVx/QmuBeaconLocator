#pragma once

#define RADIO_FREQUENCY_MIN 872000000
#define RADIO_CHANNEL_WIDTH 250000
#define RADIO_CHANNEL_COUNT 1   //Only 1 channel
#define RADIO_HOP_OFFSET 0      //Hop to the same

#ifndef RADIO_NODE_H
#define RADIO_NODE_H

#include "Arduino.h"
#include "qsp.h"
#include "variables.h"
#include "beacons.h"

class RadioNode {
    public:
        RadioNode(void);
        void init(uint8_t ss, uint8_t rst, uint8_t di0, void(*callback)(int));
        void readRssi(void);
        void readSnr(void);
        void hopFrequency(bool forward, uint8_t fromChannel, uint32_t timestamp);
        void readAndDecode(QspConfiguration_t *qsp);
        uint8_t getChannel(void);
        uint32_t getChannelEntryMillis(void);
        bool handleTxDoneState(bool hop);
        void handleTx(QspConfiguration_t *qsp);
        void configure(
            uint8_t _power, 
            long _bandwidth,
            uint8_t _spreadingFactor, 
            uint8_t _codingRate
        );
        void reset(void);
        volatile int8_t bytesToRead = -1;
        volatile uint8_t radioState = RADIO_STATE_RX;
        uint8_t rssi = 0;
        uint8_t snr = 0;
        uint8_t lastReceivedChannel = 0;
        uint8_t failedDwellsCount = 0;
        uint32_t loraBandwidth = 250000;
        uint8_t loraSpreadingFactor = 7;
        uint8_t loraCodingRate = 6;
        uint8_t loraTxPower = 10; // Defines output power of TX, defined in dBm range from 2-17
        bool canTransmit = false;
        uint8_t bindKey[4] = {0x13, 0x27, 0x42, 0x07};
    private:
        void set(
            uint8_t power, 
            long bandwidth, 
            uint8_t spreadingFactor, 
            uint8_t codingRate,
            long frequency
        );
        void flush(void);
        uint8_t _channel = 0;
        uint32_t _channelEntryMillis = 0;
        uint32_t nextTxCheckMillis = 0;
};

#endif