#include "Arduino.h"
#include "variables.h"

#ifndef QSP_H
#define QSP_H

typedef enum qspDecodingStatus {
    QSP_DECODING_STATUS_OK,
    QSP_DECODING_STATUS_ERROR,
} qspDecodingStatus_e;

void qspComputeCrc(QspConfiguration_t *qsp, uint8_t dataByte);
qspDecodingStatus_e qspDecodeIncomingFrame(
    QspConfiguration_t *qsp, 
    uint8_t incomingByte,
    uint8_t bindKey[]
);
void qspClearPayload(QspConfiguration_t *qsp);
void qspEncodeFrame(QspConfiguration_t *qsp, uint8_t buffer[], uint8_t *size, uint8_t radioChannel, uint8_t bindKey[]);

void encodePingPayload(QspConfiguration_t *qsp, uint32_t currentMicros);
void encodeBindPayload(QspConfiguration_t *qsp, uint8_t bindKey[]);

#endif