#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include "SSD1306.h"
#include <QmuTactile.h>
#include <SPI.h>
#include "variables.h"
#include "lora.h"
#include "radio_node.h"
#include "beacons.h"
#include "oled_display.h"

#ifdef ARDUINO_ESP32_DEV
    #define LORA_SS_PIN     18
    #define LORA_RST_PIN    14
    #define LORA_DI0_PIN    26

    #define SPI_SCK_PIN     5
    #define SPI_MISO_PIN    19
    #define SPI_MOSI_PIN    27
#else
    #error please select hardware
#endif

#define PIN_BUTTON_L 4
#define PIN_BUTTON_R 0

TinyGPSPlus gps;
HardwareSerial SerialGPS(1);
SSD1306  display(0x3c, 21, 22);
OledDisplay oledDisplay(&display);

QmuTactile buttonL(PIN_BUTTON_L);
QmuTactile buttonR(PIN_BUTTON_R);

RadioNode radioNode;
QspConfiguration_t qsp = {};

Beacons beacons;

#define TASK_SERIAL_RATE 500
#define TASK_LORA_READ 2 // We check for new packets only from time to time, no need to do it more often
#define TASK_LORA_TX_MS 200 // Number of ms between positio updates

uint32_t nextSerialTaskTs = 0;
uint32_t nextLoRaReadTaskTs = 0;
uint32_t nextLoRaTxTaskTs = 0;
uint32_t currentBeaconId = 0;
int8_t currentBeaconIndex = -1;

uint8_t currentDeviceMode = DEVICE_MODE_LOCATOR;
uint8_t previousDeviceMode = DEVICE_MODE_LOCATOR;

void onQspSuccess(uint8_t receivedChannel) {
    //If recide received a valid frame, that means it can start to talk
    radioNode.lastReceivedChannel = receivedChannel;
    
    radioNode.readRssi();
    radioNode.readSnr();

    uint32_t beaconId = qsp.payload[3] << 24;
    beaconId += qsp.payload[2] << 16;
    beaconId += qsp.payload[1] << 8;
    beaconId += qsp.payload[0];
    
    // Serial.print("Beacon="); Serial.println(beaconId);

    Beacon *beacon = beacons.getBeacon(beaconId);

    /*
     * Set common beacon attributes
     */
    beacon->setRssi(radioNode.rssi);
    beacon->setSnr(radioNode.snr);
    beacon->setLastContactMillis(millis());

    if (qsp.frameId == QSP_FRAME_COORDS) {
        long tmp;

        tmp = qsp.payload[4];
        tmp += qsp.payload[5] << 8;
        tmp += qsp.payload[6] << 16;
        tmp += qsp.payload[7] << 24;

        beacon->setLat(tmp / 10000000.0d);

        tmp = qsp.payload[8];
        tmp += qsp.payload[9] << 8;
        tmp += qsp.payload[10] << 16;
        tmp += qsp.payload[11] << 24;

        beacon->setLon(tmp / 10000000.0d);
    }
}

void onQspFailure() {

}

void setup()
{
    Serial.begin(115200);
	SerialGPS.begin(9600, SERIAL_8N1, 13, 15);

    buttonL.start();
    buttonR.start();

    qsp.onSuccessCallback = onQspSuccess;
    qsp.onFailureCallback = onQspFailure;

    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, LORA_SS_PIN);
    LoRa.setSPIFrequency(4E6);
    radioNode.init(LORA_SS_PIN, LORA_RST_PIN, LORA_DI0_PIN, NULL);
    radioNode.reset();
    radioNode.canTransmit = true;
    LoRa.receive();

    oledDisplay.init();
    oledDisplay.setPage(OLED_PAGE_BEACON_LIST);
}

void loop()
{
    radioNode.handleTxDoneState(false);

    if (radioNode.radioState != RADIO_STATE_TX && nextLoRaReadTaskTs < millis()) {
        int packetSize = LoRa.parsePacket();
        if (packetSize) {
            radioNode.bytesToRead = packetSize;
            radioNode.readAndDecode(&qsp);
        }

        nextLoRaReadTaskTs = millis() + TASK_LORA_READ;
    }

    /*
     * Watchdog for frame decoding stuck somewhere in the middle of a process
     */
    if (
        qsp.protocolState != QSP_STATE_IDLE &&
        qsp.frameDecodingStartedAt + QSP_MAX_FRAME_DECODE_TIME < millis()
    ) {
        qsp.protocolState = QSP_STATE_IDLE;
    }

    buttonL.loop();
    buttonR.loop();

    //TODO remove when ready
    if (SerialGPS.available()) {
        while (SerialGPS.available() > 0) {
            gps.encode(SerialGPS.read());
        }
    }

    /*
     * Right button long press changes the device mode!
     */
    if (buttonR.getState() == TACTILE_STATE_LONG_PRESS) {
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
        currentDeviceMode == DEVICE_MODE_BEACON &&
        nextLoRaTxTaskTs > millis() && 
        gps.satellites.value() > 4
    ) {

        // Prepare packet and send position

        nextLoRaTxTaskTs = millis() + TASK_LORA_TX_MS;
    }

    if (nextSerialTaskTs < millis()) {
        if (currentBeaconIndex >= 0) {
            // Beacon *beacon = beacons.get(currentBeaconIndex);
            // Serial.print("LAT=");  Serial.println(gps.location.lat(), 6);
            // Serial.print("LONG="); Serial.println(gps.location.lng(), 6);
            // Serial.print("ALT=");  Serial.println(gps.altitude.meters());
            // Serial.print("RSSI=");  Serial.println(beacon->getRssi());
            // Serial.print("SNR=");  Serial.println(beacon->getSnr());
        }
        nextSerialTaskTs = millis() + TASK_SERIAL_RATE;
    }

    if (currentBeaconIndex == -1 && beacons.count() > 0) {
        currentBeaconIndex = 0;
        currentBeaconId = beacons.get(currentBeaconIndex)->getId();
    }

    oledDisplay.loop();
}
