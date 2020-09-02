#include "Arduino.h"
#include "variables.h"

void qspComputeCrc(QspConfiguration_t *qsp, uint8_t dataByte);
void qspDecodeIncomingFrame(
    QspConfiguration_t *qsp, 
    uint8_t incomingByte,
    uint8_t bindKey[]
);
void qspClearPayload(QspConfiguration_t *qsp);
void qspEncodeFrame(QspConfiguration_t *qsp, uint8_t buffer[], uint8_t *size, uint8_t radioChannel, uint8_t bindKey[]);

void encodePingPayload(QspConfiguration_t *qsp, uint32_t currentMicros);
void encodeBindPayload(QspConfiguration_t *qsp, uint8_t bindKey[]);