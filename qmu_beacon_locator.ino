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

uint32_t nextSerialTaskTs = 0;
uint32_t nextLoRaTaskTs = 0;
uint32_t currentBeaconId = 0;
int8_t currentBeaconIndex = -1;

void onQspSuccess(uint8_t receivedChannel) {
    //If recide received a valid frame, that means it can start to talk
    radioNode.lastReceivedChannel = receivedChannel;
    
    radioNode.readRssi();
    radioNode.readSnr();

    uint32_t beaconId = qsp.payload[3] << 24;
    beaconId += qsp.payload[2] << 16;
    beaconId += qsp.payload[1] << 8;
    beaconId += qsp.payload[0];
    
    Serial.print("Beacon="); Serial.println(beaconId);

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
    oledDisplay.page(OLED_PAGE_LIST);
}

void loop()
{
    if (nextLoRaTaskTs < millis()) {
        int packetSize = LoRa.parsePacket();
        if (packetSize) {

            radioNode.bytesToRead = packetSize;
            radioNode.readAndDecode(
                &qsp,
                beacons
            );
        }

        nextLoRaTaskTs = millis() + TASK_LORA_READ;
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

    if (gps.satellites.value() > 4) {

    }
/*
    if (nextOledTaskTs < millis()) {
        display.clear();
        display.drawString(0, 0, "Lat: " + String(gps.location.lat(), 5));
        display.drawString(0, 10, "Lon: " + String(gps.location.lng(), 5));
        display.drawString(0, 20, "Sat: " + String(gps.satellites.value(), 5));

        Beacon *beacon = beacons.getBeacon(currentBeaconId);

        display.drawString(0, 30, "Lat: " + String(beacon->getLat(), 5));
        display.drawString(0, 40, "Lon: " + String(beacon->getLon(), 5));

        double dst = TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), beacon->getLat(), beacon->getLon());
        display.drawString(0, 50, "Dst: " + String(dst, 1));

        display.display();

        nextOledTaskTs = millis() + TASK_OLED_RATE;
    }
*/
    if (nextSerialTaskTs < millis()) {
        // Beacon *beacon = beacons.getBeacon(currentBeaconId);
        // Serial.print("LAT=");  Serial.println(gps.location.lat(), 6);
        // Serial.print("LONG="); Serial.println(gps.location.lng(), 6);
        // Serial.print("ALT=");  Serial.println(gps.altitude.meters());
        // Serial.print("RSSI=");  Serial.println(beacon->getRssi());
        // Serial.print("SNR=");  Serial.println(beacon->getSnr());

        nextSerialTaskTs = millis() + TASK_SERIAL_RATE;
    }

    if (currentBeaconIndex == -1 && beacons.count() > 0) {
        currentBeaconIndex = 0;
    }

    oledDisplay.loop();
}
