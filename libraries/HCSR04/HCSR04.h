/*
 * HCSR04.h - Library for HC-SR04 Ultrasonic Ranging Module
 * Richard Hwang
 * Reference:
 *  https://docs.google.com/document/d/
 *    1Y-yZnNhMYy7rwhAgyL_pfa39RsB-x2qR4vP8saG73rE/edit
 *
 */

#ifndef HCSR04_h
#define HCSR04_h

#include "Arduino.h"

class HCSR04 {
  public:
    HCSR04(uint8_t trig, uint8_t echo);
    uint16_t range(int unit);

  private:
    uint8_t TRIG_PIN;
    uint8_t ECHO_PIN;
};

#endif
