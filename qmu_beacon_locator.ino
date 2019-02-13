#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include "SSD1306.h"
#include <QmuTactile.h>
#include <SPI.h>
#include "variables.h"
#include "lora.h"
#include "radio_node.h"

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

QmuTactile buttonL(PIN_BUTTON_L);
QmuTactile buttonR(PIN_BUTTON_R);

RadioNode radioNode;
QspConfiguration_t qsp = {};
BeaconState_t beaconState = {};

#define TASK_SERIAL_RATE 500
#define TASK_OLED_RATE 200

uint32_t nextSerialTaskTs = 0;
uint32_t nextOledTaskTs = 0;

uint32_t zzz = 0;

void setup()
{
    Serial.begin(115200);
	SerialGPS.begin(9600, SERIAL_8N1, 13, 15);
    
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);

    buttonL.start();
    buttonR.start();

    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, LORA_SS_PIN);
    radioNode.init(LORA_SS_PIN, LORA_RST_PIN, LORA_DI0_PIN, NULL);
    radioNode.reset();
    radioNode.canTransmit = true;
    LoRa.receive();
}

void loop()
{

    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        Serial.println(packetSize);

        for (int i = 0; i < packetSize; i++) { 
            Serial.print((uint8_t)LoRa.fastRead());
            Serial.print(" ");
        }
        Serial.println(" ");
        LoRa.sleep();
        LoRa.receive();
    }

    buttonL.loop();
    buttonR.loop();

    if (buttonL.getState() == TACTILE_STATE_SHORT_PRESS) {
        zzz = 7;
    }

    if (buttonR.getState() == TACTILE_STATE_SHORT_PRESS) {
        zzz = 13;
    }

	while (SerialGPS.available() > 0) {
        gps.encode(SerialGPS.read());
    }

    if (gps.satellites.value() > 4) {

    }

    if (nextOledTaskTs < millis()) {
        display.clear();
        display.drawString(0, 0, "Lat: " + String(gps.location.lat(), 5));
        display.drawString(0, 10, "Lon: " + String(gps.location.lng(), 5));
        display.drawString(0, 20, String(zzz));
        display.display();

        nextOledTaskTs = millis() + TASK_OLED_RATE;
    }

    if (nextSerialTaskTs < millis()) {

        // Serial.print("LAT=");  Serial.println(gps.location.lat(), 6);
        // Serial.print("LONG="); Serial.println(gps.location.lng(), 6);
        // Serial.print("ALT=");  Serial.println(gps.altitude.meters());
        // Serial.print("Sats=");  Serial.println(gps.satellites.value());

        nextSerialTaskTs = millis() + TASK_SERIAL_RATE;
    }
}
