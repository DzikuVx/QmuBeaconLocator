#include "Arduino.h"

#pragma once

#ifndef VARIABLES_H
#define VARIABLES_H

#define RADIO_STATE_TX 1
#define RADIO_STATE_RX 2

#define QSP_PAYLOAD_LENGTH 32

#define MIN_PACKET_SIZE 3 //Min theorethical size of valid packet 
#define MAX_PACKET_SIZE 34 //Max theorethical size of valid packet
#define NO_DATA_TO_READ -1

enum qspFrames {
    QSP_FRAME_IDENT = 0,
    QSP_FRAME_COORDS,
    QSP_FRAME_COUNT
};

static const uint8_t qspFrameLengths[QSP_FRAME_COUNT] = {
    9 //QSP_FRAME_RC_DATA
};

enum dataStates {
    QSP_STATE_IDLE,
    QSP_STATE_FRAME_TYPE_RECEIVED,
    QSP_STATE_PAYLOAD_RECEIVED,
    QSP_STATE_CRC_RECEIVED
};

struct BeaconState_t {
    uint8_t rssi = 0;
    uint8_t snr = 0;
    uint8_t rxVoltage = 0;
    uint8_t a1Voltage = 0;
    uint8_t a2Voltage = 0;
    uint8_t flags = 0;
    int16_t indicatedRssi = 0;
    double lat = 0;
    double lon = 0; 
    double alt = 0;
};

struct QspConfiguration_t {
    uint8_t protocolState = QSP_STATE_IDLE;
    uint8_t crc = 0;
    uint8_t payload[QSP_PAYLOAD_LENGTH] = {0};
    uint8_t payloadLength = 0;
    uint8_t frameToSend = 0;
    uint8_t frameId = 0;
    uint32_t lastFrameReceivedAt[QSP_FRAME_COUNT] = {0};
    uint32_t anyFrameRecivedAt = 0;
    void (* onSuccessCallback)(QspConfiguration_t*, BeaconState_t*, uint8_t receivedChannel);
    void (* onFailureCallback)(QspConfiguration_t*, BeaconState_t*);    
    uint32_t frameDecodingStartedAt = 0;
    uint32_t lastTxSlotTimestamp = 0;
    bool transmitWindowOpen = false;
};

#endif