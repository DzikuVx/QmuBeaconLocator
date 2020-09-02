#include "Arduino.h"
#include "variables.h"

uint8_t crc8_dvb_s2(uint8_t crc, uint8_t a)
{
    crc ^= a;
    for (int ii = 0; ii < 8; ++ii) {
        if (crc & 0x80) {
            crc = (crc << 1) ^ 0xD5;
        } else {
            crc = crc << 1;
        }
    }
    return crc;
}

void qspComputeCrc(QspConfiguration_t *qsp, uint8_t dataByte)
{
    qsp->crc = crc8_dvb_s2(qsp->crc, dataByte);
}

void qspClearPayload(QspConfiguration_t *qsp)
{
    for (uint8_t i = 0; i < QSP_PAYLOAD_LENGTH; i++)
    {
        qsp->payload[i] = 0;
    }
    qsp->payloadLength = 0;
}

/**
 * Init CRC with salt based on 4 byte bind key
 */
void qspInitCrc(QspConfiguration_t *qsp, uint8_t bindKey[]) {
    qsp->crc = 0;
    for (uint8_t i = 0; i < 4; i++) {
        qspComputeCrc(qsp, bindKey[i]);
    }
}

void qspDecodeIncomingFrame(
    QspConfiguration_t *qsp, 
    uint8_t incomingByte,
    uint8_t bindKey[]
) {
    static uint8_t frameId;
    static uint8_t payloadLength;
    static uint8_t receivedPayload;
    static uint8_t receivedChannel;

    if (qsp->protocolState == QSP_STATE_IDLE)
    {
        qspInitCrc(qsp, bindKey);
        qspClearPayload(qsp);
        receivedPayload = 0;
        qsp->frameDecodingStartedAt = millis();

        //Frame ID and payload length
        qspComputeCrc(qsp, incomingByte);

        qsp->frameId = (incomingByte >> 4) & 0x0f;
        payloadLength = qspFrameLengths[qsp->frameId];
        receivedChannel = incomingByte & 0x0f;
        qsp->protocolState = QSP_STATE_FRAME_TYPE_RECEIVED;
    }
    else if (qsp->protocolState == QSP_STATE_FRAME_TYPE_RECEIVED)
    {
        if (receivedPayload >= QSP_PAYLOAD_LENGTH) {
            qsp->protocolState = QSP_STATE_IDLE;
        }

        //Now it's time for payload
        qspComputeCrc(qsp, incomingByte);
        qsp->payload[receivedPayload] = incomingByte;

        receivedPayload++;

        if (receivedPayload == payloadLength)
        {
            qsp->protocolState = QSP_STATE_PAYLOAD_RECEIVED;
            qsp->payloadLength = payloadLength;
        }
    }
    else if (qsp->protocolState == QSP_STATE_PAYLOAD_RECEIVED)
    {
        if (qsp->crc == incomingByte) {
            //CRC is correct
            qsp->onSuccessCallback(receivedChannel);
        } else {
            qsp->onFailureCallback();
        }

        // In both cases switch to listening for next preamble
        qsp->protocolState = QSP_STATE_IDLE;
    }
}

/**
 * Encode frame is corrent format and write to hardware
 */
void qspEncodeFrame(
    QspConfiguration_t *qsp, 
    uint8_t buffer[], 
    uint8_t *size, 
    uint8_t radioChannel,
    uint8_t bindKey[]
) {
    //Salt CRC with bind key
    qspInitCrc(qsp, bindKey);

    //Write frame type and length
    // We are no longer sending payload length, so 4 bits are now free for other usages
    // uint8_t data = qsp->payloadLength & 0x0f;
    uint8_t data = radioChannel;
    data |= (qsp->frameToSend << 4) & 0xf0;
    qspComputeCrc(qsp, data);
    buffer[0] = data;

    for (uint8_t i = 0; i < qsp->payloadLength; i++)
    {
        qspComputeCrc(qsp, qsp->payload[i]);
        buffer[i + 1] = qsp->payload[i];
    }

    buffer[qsp->payloadLength + 1] = qsp->crc;
    *size = qsp->payloadLength + 2; //Total length of QSP frame
}