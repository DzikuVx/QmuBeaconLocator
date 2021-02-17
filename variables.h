#include "Arduino.h"

#pragma once

#ifndef VARIABLES_H
#define VARIABLES_H

#define MAX_BEACONS 32

#define RADIO_STATE_TX 1
#define RADIO_STATE_RX 2

#define QSP_PAYLOAD_LENGTH 32
#define QSP_MAX_FRAME_DECODE_TIME 10

#define MIN_PACKET_SIZE 3 //Min theorethical size of valid packet 
#define MAX_PACKET_SIZE 34 //Max theorethical size of valid packet
#define NO_DATA_TO_READ -1

#define GPS_SATS_FOR_FIX 5

enum qspFrames {
    QSP_FRAME_IDENT = 0,
    QSP_FRAME_COORDS,
    QSP_FRAME_MISC,
    QSP_FRAME_COUNT
};

//Length of the payload
static const uint8_t qspFrameLengths[QSP_FRAME_COUNT] = {
    4, //QSP_FRAME_IDENT
    20, //QSP_FRAME_COORDS
    13 //QSP_FRAME_MISC
};

enum coordsFrameFlags {
    POSITION_FLAG_NONE              = 0,        //  0
    POSITION_FLAG_HEADING_VALID     = 1 << 0,   //  1
    POSITION_FLAG_POSITION_VALID    = 1 << 1,   //  2
    POSITION_FLAG_ALTITUDE_VALID    = 1 << 2,   //  4   
};

enum coordsFrameActions {
    POSITION_ACTION_NONE                = 0,
    POSITION_ACTION_FOLLOW              = 1,
    POSITION_ACTION_ORBIT               = 2,
    POSITION_ACTION_FORMATION_AHEAD     = 3,
    POSITION_ACTION_FORMATION_BEHIND    = 4,
    POSITION_ACTION_FORMATION_LEFT      = 5,
    POSITION_ACTION_FORMATION_RIGHT     = 6,
    POSITION_ACTION_LOOK_AT_ME          = 7,
};

enum dataStates {
    QSP_STATE_IDLE,
    QSP_STATE_FRAME_TYPE_RECEIVED,
    QSP_STATE_PAYLOAD_RECEIVED,
    QSP_STATE_CRC_RECEIVED
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
    void (* onSuccessCallback)(uint8_t receivedChannel);
    void (* onFailureCallback)(void);    
    uint32_t frameDecodingStartedAt = 0;
    uint32_t lastTxSlotTimestamp = 0;
    bool transmitWindowOpen = false;
};

#endif