#include "Arduino.h"
#include "HCSR04.h"

HCSR04::HCSR04(uint8_t trig, uint8_t echo) {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  TRIG_PIN = trig;
  ECHO_PIN = echo;
}

/* Returns the current range, in cm if unit == 0 else inch. */
uint16_t HCSR04::range(int unit) {

  // Send trigger.
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read.
  unsigned long echo_width_us = pulseIn(ECHO_PIN, HIGH);

  // Convert.
  if (unit == 0) {
    return (uint16_t) echo_width_us / 58;
  } else {
    return (uint16_t) echo_width_us / 148;
  }
}
