#include "TinyGPSpp.h"
#include <HardwareSerial.h>
#include "SSD1306.h"
#include "QmuTactile.h"
#include <SPI.h>
#include "variables.h"
#include "lora.h"
#include "radio_node.h"
#include "beacons.h"
#include "oled_display.h"
#include "config_node.h"
#include "utils.h"
#include "device_node.h"

#define LORA_TX_POWER 10
#define LORA_BANDWIDTH 500000
#define LORA_SPREADING_FACTOR 7
#define LORA_CODING_RATE 6

//Target
// #define ARDUINO_TTGO_TBEAM_ESP32

#ifdef ARDUINO_TTGO_TBEAM_ESP32
    #define LORA_SS_PIN     18
    #define LORA_RST_PIN    23
    #define LORA_DI0_PIN    26

    #define SPI_SCK_PIN     5
    #define SPI_MISO_PIN    19
    #define SPI_MOSI_PIN    27

    #define PIN_BUTTON_L 38
    // #define PIN_BUTTON_R 0

    #define I2C_SDA_PIN 21
    #define I2C_SCL_PIN 22

    #define GPS_SERIAL_TX_PIN 12
    #define GPS_SERIAL_RX_PIN 34
#elif defined(ARDUINO_ESP32_DEV)
    #define LORA_SS_PIN     18
    #define LORA_RST_PIN    14
    #define LORA_DI0_PIN    26

    #define SPI_SCK_PIN     5
    #define SPI_MISO_PIN    19
    #define SPI_MOSI_PIN    27

    #define PIN_BUTTON_L 4
    #define PIN_BUTTON_R 0

    #define I2C_SDA_PIN 21
    #define I2C_SCL_PIN 22

    #define GPS_SERIAL_TX_PIN 15
    #define GPS_SERIAL_RX_PIN 13
#else
    #error please select hardware
#endif

TinyGPSPlus gps;
HardwareSerial SerialGPS(1);
SSD1306  display(0x3c, I2C_SDA_PIN, I2C_SCL_PIN);
OledDisplay oledDisplay(&display);

QmuTactile buttonMain(PIN_BUTTON_L);

#ifdef PIN_BUTTON_R
QmuTactile buttonR(PIN_BUTTON_R);
#endif

RadioNode radioNode;
QspConfiguration_t qsp = {};
Beacons beacons;

ConfigNode configNode;

#define TASK_SERIAL_RATE 500
#define TASK_LORA_READ 1 // We check for new packets only from time to time, no need to do it more often
#define TASK_LORA_TX_MS 400 // Number of ms between positio updates

DeviceNode deviceNode(TASK_LORA_TX_MS);

uint32_t nextSerialTaskTs = 0;
uint32_t nextLoRaReadTaskTs = 0;

void onQspSuccess(uint8_t receivedChannel) {
    //If recide received a valid frame, that means it can start to talk
    radioNode.lastReceivedChannel = receivedChannel;
    
    radioNode.readRssi();
    radioNode.readSnr();

    uint32_t beaconId = qsp.payload[3] << 24;
    beaconId += qsp.payload[2] << 16;
    beaconId += qsp.payload[1] << 8;
    beaconId += qsp.payload[0];
    
    Beacon *beacon = beacons.getBeacon(beaconId);

    /*
     * Set common beacon attributes
     */
    beacon->setRssi(radioNode.rssi);
    beacon->setSnr(radioNode.snr);
    beacon->setLastContactMillis(millis());

    long tmp;
    if (qsp.frameId == QSP_FRAME_COORDS) {

        //We have a valid position data
        if (qsp.payload[19] & POSITION_FLAG_POSITION_VALID) {
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

        if (qsp.payload[19] & POSITION_FLAG_ALTITUDE_VALID) {
            tmp = qsp.payload[12];
            tmp += qsp.payload[13] << 8;
            tmp += qsp.payload[14] << 16;
            tmp += qsp.payload[15] << 24;

            beacon->setAlt(tmp / 100.0d);
        }

        if (qsp.payload[19] & POSITION_FLAG_ALTITUDE_VALID) {
            tmp = qsp.payload[12];
            tmp += qsp.payload[13] << 8;
            tmp += qsp.payload[14] << 16;
            tmp += qsp.payload[15] << 24;

            beacon->setAlt(tmp / 100.0d);
        }

        if (qsp.payload[19] & POSITION_FLAG_HEADING_VALID) {
            tmp = qsp.payload[16];
            tmp += qsp.payload[17] << 8;

            beacon->setCourse(tmp);
        }

        beacon->setAction(qsp.payload[18]);
        beacon->setFlags(qsp.payload[19]);
    } else if (qsp.frameId == QSP_FRAME_MISC) {

        //HDOP
        tmp = qsp.payload[4];
        tmp += qsp.payload[5] << 8;
        tmp += qsp.payload[6] << 16;
        tmp += qsp.payload[7] << 24;
        beacon->setHdop(tmp);

        //Speed
        tmp = qsp.payload[8];
        tmp += qsp.payload[9] << 8;
        tmp += qsp.payload[10] << 16;
        tmp += qsp.payload[11] << 24;
        beacon->setSpeed(tmp / 100.0f);

        beacon->setSats(qsp.payload[12]);
    }
}

void onQspFailure() {
    Serial.println("Failure to decode QSP frame");
}

void setup()
{
    Serial.begin(115200);
	SerialGPS.begin(9600, SERIAL_8N1, GPS_SERIAL_RX_PIN, GPS_SERIAL_TX_PIN);

    configNode.begin();
    randomSeed(analogRead(A4));
    configNode.seed();
    configNode.beaconId = configNode.loadBeaconId();

    deviceNode.begin();

    buttonMain.start();
#ifdef PIN_BUTTON_R
    buttonR.start();
#endif

    qsp.onSuccessCallback = onQspSuccess;
    qsp.onFailureCallback = onQspFailure;

    radioNode.configure(
        LORA_TX_POWER, 
        LORA_BANDWIDTH, 
        LORA_SPREADING_FACTOR, 
        LORA_CODING_RATE
    );

    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, LORA_SS_PIN);
    LoRa.setSPIFrequency(2E6);
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

    buttonMain.loop();

#ifdef PIN_BUTTON_R
    buttonR.loop();
#endif

    //TODO remove when ready
    if (SerialGPS.available()) {
        while (SerialGPS.available() > 0) {
            gps.encode(SerialGPS.read());
        }
    }

    deviceNode.execute();
    deviceNode.processInputs();

    if (deviceNode.getDeviceMode() == DEVICE_MODE_LOCATOR) {
        oledDisplay.setPage(OLED_PAGE_BEACON_LIST);
    } else if (deviceNode.getDeviceMode() == DEVICE_MODE_LOOK_AT_ME) {
        oledDisplay.setPage(OLED_PAGE_LOOK_AT_ME);
    } else {
        oledDisplay.setPage(OLED_PAGE_I_AM_A_BEACON);
    }

    if (nextSerialTaskTs < millis()) {
        if (beacons.currentBeaconIndex >= 0) {
            Beacon *beacon = beacons.get(beacons.currentBeaconIndex);
            Serial.print("LAT     = ");  Serial.println(beacon->getLat(), 6);
            Serial.print("LONG    = "); Serial.println(beacon->getLon(), 6);
            Serial.print("COURSE  = "); Serial.println(beacon->getCourse(), 3);
            Serial.print("ALT     = ");  Serial.println(beacon->getAlt());
            Serial.print("Action  = ");  Serial.println(beacon->getActionRaw());
            Serial.print("Flags   = ");  Serial.println(beacon->getFlagsRaw());
            Serial.print("Sats    = ");  Serial.println(beacon->getSats());
            Serial.print("HDOP    = ");  Serial.println(beacon->getHdop());
            Serial.print("Speed   = ");  Serial.println(beacon->getSpeed());
            Serial.print("RSSI    = ");  Serial.println(beacon->getRssi());
            Serial.print("SNR     = ");  Serial.println(beacon->getSnr());
        }
        nextSerialTaskTs = millis() + TASK_SERIAL_RATE;
    }

    if (beacons.currentBeaconIndex == -1 && beacons.count() > 0) {
        beacons.currentBeaconIndex = 0;
        beacons.currentBeaconId = beacons.get(beacons.currentBeaconIndex)->getId();
    }

    oledDisplay.loop();
}
