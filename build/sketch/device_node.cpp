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

    if (
        currentDeviceMode == DEVICE_MODE_LOCATOR && 
        beacons.count() > 1 &&
        buttonMain.getState() == TACTILE_STATE_SHORT_PRESS
    ) {
        beacons.currentBeaconIndex++;
        if (beacons.currentBeaconIndex == beacons.count()) {
            beacons.currentBeaconIndex = 0;
        }

        beacons.currentBeaconId = beacons.get(beacons.currentBeaconIndex)->getId();
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
        if (random(1, 100) < 20) {
            frameToSend = QSP_FRAME_MISC;
        } else {
            frameToSend = QSP_FRAME_COORDS;
        }

        long writeValue; //Just a temporary variable

        if (frameToSend == QSP_FRAME_COORDS) {

            const bool havePosition = (gps.satellites.value() > 5) ? true : false;
            const bool haveCourse = (gps.speed.kmph() > 5) ? true : false;

            int32ToBuf(qsp.payload, 0, configNode.beaconId);

            if (havePosition) {
                writeValue = gps.location.lat() * 10000000.0d;
            } else {
                writeValue = 0;
            }
            int32ToBuf(qsp.payload, 4, writeValue);

            if (havePosition) {
                writeValue = gps.location.lng() * 10000000.0d;
            }else {
                writeValue = 0;
            }
            int32ToBuf(qsp.payload, 8, writeValue);

            if (havePosition) {
                writeValue = gps.altitude.meters() * 100.0d; //Altitude in cm    
            } else {
                writeValue = 0;
            }
            int32ToBuf(qsp.payload, 12, writeValue);

             if (haveCourse) {
                writeValue = gps.course.deg(); //Course in degrees   
            } else {
                writeValue = 0;
            }
            int16ToBuf(qsp.payload, 16, writeValue);

            //Position 18 is for action
            if (currentDeviceMode == DEVICE_MODE_BEACON) {
                qsp.payload[18] = POSITION_ACTION_NONE;
            } else {
                qsp.payload[18] = POSITION_ACTION_NONE;
            }   

            //Position 19 is for flags
            qsp.payload[19] = POSITION_FLAG_NONE;

            if (havePosition) {
                qsp.payload[19] |= POSITION_FLAG_POSITION_VALID;
                qsp.payload[19] |= POSITION_FLAG_ALTITUDE_VALID;
            }

            if (haveCourse) {
                qsp.payload[19] |= POSITION_FLAG_HEADING_VALID;
            }

            qsp.frameToSend = QSP_FRAME_COORDS;
            qsp.payloadLength = 20;
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