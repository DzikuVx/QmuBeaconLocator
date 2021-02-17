#ifndef OLED_DISPLAY
#define OLED_DISPLAY

#include "SSD1306.h"
#include <TinyGPS++.h>
#include "beacons.h"
#include "device_node.h"

enum txOledPages {
    OLED_PAGE_NONE,
    OLED_PAGE_BEACON_LIST,
    OLED_PAGE_I_AM_A_BEACON,
    OLED_PAGE_LOOK_AT_ME,
};

#define OLED_COL_COUNT 64
#define OLED_DISPLAY_PAGE_COUNT 2

extern TinyGPSPlus gps;
extern Beacons beacons;
extern DeviceNode deviceNode;

class OledDisplay {
    public:
        OledDisplay(SSD1306 *display);
        void init();
        void loop();
        void setPage(uint8_t page);
    private:
        SSD1306 *_display;
        void renderPageBeaconList();
        void renderPageIamBeacon();
        void renderPageLookAtMe();
        void renderHeader(String title);
        void page();
        uint8_t _page = OLED_PAGE_NONE;
        bool _forceDisplay = false;
};


#endif