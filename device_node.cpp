#include "device_node.h"
#include "utils.h"

DeviceNode::DeviceNode(uint16_t _taskTxMs) {
    taskTxMs = _taskTxMs;
}

void DeviceNode::processInputs(void) {
    /*
     * Main (LEFT) button long press changes the device mode!
     */
    if (buttonMain.getState() == TACTILE_STATE_LONG_PRESS) {
        previousDeviceMode = currentDeviceMode;
        currentDeviceMode++;
        if (currentDeviceMode == DEVICE_MODE_LAST) {
            currentDeviceMode = DEVICE_MODE_LOCATOR;
        }
    }

    /*
     * Handle device mode changes
     */
    if (currentDeviceMode != previousDeviceMode) {

        if (currentDeviceMode == DEVICE_MODE_LOCATOR) {
            oledDisplay.setPage(OLED_PAGE_BEACON_LIST);
        } else {
            oledDisplay.setPage(OLED_PAGE_I_AM_A_BEACON);
        }

        previousDeviceMode = currentDeviceMode;
    }
}

void DeviceNode::execute(void) {
    
    bool transmitPayload = false;

    if (
        currentDeviceMode == DEVICE_MODE_BEACON &&
        nextLoRaTxTaskTs < millis() && 
        qsp.protocolState == QSP_STATE_IDLE && 
        radioNode.radioState == RADIO_STATE_RX
    ) {

        // Prepare packet and send position
        qspClearPayload(&qsp);

        int8_t frameToSend;
        if (gps.satellites.value() < 6) {
            frameToSend = QSP_FRAME_IDENT;
        } else if (random(1, 100) < 25) {
            frameToSend = QSP_FRAME_MISC;
        } else {
            frameToSend = QSP_FRAME_COORDS;
        }

        long writeValue; //Just a temporary variable

        if (frameToSend == QSP_FRAME_IDENT) {
            int32ToBuf(qsp.payload, 0, configNode.beaconId);

            qsp.payloadLength = 4;
            qsp.frameToSend = QSP_FRAME_IDENT;
        } else if (frameToSend == QSP_FRAME_COORDS) {
            int32ToBuf(qsp.payload, 0, configNode.beaconId);

            writeValue = gps.location.lat() * 10000000.0d;
            int32ToBuf(qsp.payload, 4, writeValue);

            writeValue = gps.location.lng() * 10000000.0d;
            int32ToBuf(qsp.payload, 8, writeValue);

            qsp.frameToSend = QSP_FRAME_COORDS;
            qsp.payloadLength = 12;
        } else if (frameToSend == QSP_FRAME_MISC) {
            int32ToBuf(qsp.payload, 0, configNode.beaconId);

            writeValue = gps.hdop.value();
            int32ToBuf(qsp.payload, 4, writeValue);

            writeValue = gps.speed.mps() * 100.0d;
            int32ToBuf(qsp.payload, 8, writeValue);

            writeValue = gps.altitude.meters() * 100.0d;
            int32ToBuf(qsp.payload, 12, writeValue);

            qsp.payload[16] = gps.satellites.value();

            qsp.frameToSend = QSP_FRAME_MISC;
            qsp.payloadLength = 17;
        }

        transmitPayload = true;

        nextLoRaTxTaskTs = millis() + taskTxMs + random(20, 40);
    }

    if (transmitPayload)
    {
        radioNode.handleTx(&qsp);
    }

}