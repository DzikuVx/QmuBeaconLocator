#pragma once

#ifndef QMU_TACTILE_H
#define QMU_TACTILE_H

#include "Arduino.h"

enum tactileStateFlags {
    TACTILE_STATE_NONE,
    TACTILE_STATE_SHORT_PRESS,
    TACTILE_STATE_LONG_PRESS,
    TACTILE_STATE_PRESSING
};

#define TACTILE_MIN_PRESS_TIME 50
#define TACTILE_LONG_PRESS_TIME 1000
#define TACTILE_PRESSING_TIME 400

class QmuTactile {
  public:
  	QmuTactile(uint8_t pin);
    void loop(void);
    void start(void);
    uint8_t getState(void);
  private:
    uint8_t _pin;
    uint8_t _previousPinState = HIGH;
    uint32_t _pressMillis = 0;
    uint8_t _state = TACTILE_STATE_NONE;
    uint32_t _nextPressingEvent = 0;
};

#endif