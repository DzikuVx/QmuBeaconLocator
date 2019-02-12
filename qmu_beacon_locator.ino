#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include "SSD1306.h"
#include <QmuTactile.h>

struct BeaconState_t {
    double lat = 0;
    double lon = 0; 
    double alt = 0;
};

#define PIN_BUTTON_L 4
#define PIN_BUTTON_R 0

TinyGPSPlus gps;
HardwareSerial SerialGPS(1);
SSD1306  display(0x3c, 21, 22);

QmuTactile buttonL(PIN_BUTTON_L);
QmuTactile buttonR(PIN_BUTTON_R);

#define TASK_SERIAL_RATE 100
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
}

void loop()
{

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

        Serial.print("LAT=");  Serial.println(gps.location.lat(), 6);
        Serial.print("LONG="); Serial.println(gps.location.lng(), 6);
        Serial.print("ALT=");  Serial.println(gps.altitude.meters());
        Serial.print("Sats=");  Serial.println(gps.satellites.value());

        nextSerialTaskTs = millis() + TASK_SERIAL_RATE;
    }
}
