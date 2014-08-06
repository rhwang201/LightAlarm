#include <Wire.h>
#include "RTClib.h"
#include "LPD8806.h"
#include "HCSR04.h"
#include "SPI.h"

uint8_t DEBUG_LA = 1;

uint8_t LED_data_pin = 22,
        LED_clock_pin = 24;

uint8_t n_LEDS = 32,
        num_LEDS_on = 0,
        num_group_on = 0;


LPD8806 strip = LPD8806(n_LEDS, LED_data_pin, LED_clock_pin);
RTC_DS1307 rtc;


uint32_t round_up(uint32_t num, uint8_t multiple);

uint32_t desired_start_min = 1,
         desired_start_time = desired_start_min * 60,
         start_time = round_up(desired_start_time, n_LEDS),
         start_delay = start_time / n_LEDS,
         desired_stop_min = 3,
         stop_time = desired_stop_min * 60,
         HOUR = 7,
         MIN = 50,
         ALARM_TIME = HOUR * 60 * 60 + MIN * 60,
         WAIT_THRESH =  start_time + 120;

enum color {
  WHITE,
  RED,
  ORANGE,
  YELLOW,
  MAGENTA,
  GREEN,
  CYAN,
  BLUE,
  RAND,
  BLACK
};
uint8_t num_colors = 9;
uint32_t WHITE_COLOR = strip.Color(127, 127, 127),
         RED_COLOR = strip.Color(127,0,0),
         ORANGE_COLOR = strip.Color(127,63,0),
         YELLOW_COLOR = strip.Color(127,127,0),
         MAGENTA_COLOR = strip.Color(127,0,127),
         GREEN_COLOR = strip.Color(0,127,0),
         CYAN_COLOR = strip.Color(0,127,127),
         BLUE_COLOR = strip.Color(0,0,127),
         BLACK_COLOR = strip.Color(0,0,0);


uint32_t DateTime_to_secs(DateTime d);
void clear_strip();
void log_time(uint8_t hr, uint8_t min, uint8_t sec);
void log_times(uint32_t now, uint32_t alarm, uint32_t start);
void next_LED_on();
void dither(uint32_t c, uint8_t wait);
void random_dither();



/* Start and update strip. */
void setup() {
  strip.begin();
  strip.show();

  Serial.begin(57600);
  Wire.begin();
  rtc.begin();

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
  }

  DateTime now = rtc.now();
  //ALARM_TIME =  DateTime_to_secs(now) + start_time + 20; // TODO
}


/* Gradually increase light if alarm time. */
void loop() {
  DateTime dt_now = rtc.now();
  uint32_t now = DateTime_to_secs(dt_now);
  uint8_t hr = dt_now.hour(),
          min = dt_now.minute(),
          sec = dt_now.second();

  // Waiting to light up
  if (now + start_time < ALARM_TIME) {
    uint32_t diff = ALARM_TIME - now - start_time;

    if (DEBUG_LA) {
      log_time(hr, min, sec);
      Serial.print("SHORT");
      log_times(now, ALARM_TIME, start_time);
    }

    clear_strip();
    delay(1000);

  // Gradually turn on LEDS, every fourth one
  } else if (now < ALARM_TIME) {
    if (DEBUG_LA) {
      log_time(hr, min, sec);
      Serial.print("ON ");
      Serial.print(num_LEDS_on);
      log_times(now, ALARM_TIME, start_time);
    }

    next_LED_on();
    delay(start_delay * 1000);

  // Stay on
  } else if (now < ALARM_TIME + stop_time) {
    if (DEBUG_LA) {
      log_time(hr, min, sec);
      Serial.print("WAKE ");
      log_times(now, ALARM_TIME, start_time);
    }

    random_dither();

  // Turn off
  } else {
    if (DEBUG_LA) {
      log_time(hr, min, sec);
      Serial.print("DONE, sleeping ");
      log_times(now, ALARM_TIME, start_time);
    }

    clear_strip();
    delay(1000);
  }
}


/* Round up num to the nearest multiple of multiple. */
uint32_t round_up(uint32_t num, uint8_t multiple) {
  uint8_t remainder = num % multiple;
  if (remainder == 0) {
    return num;
  } else {
    return num + multiple - remainder;
  }
}

/* hh/mm/ss -> seconds */
uint32_t DateTime_to_secs(DateTime d) {
  return ((uint32_t) d.hour()) * 60 * 60 + ((uint32_t) d.minute()) * 60
    + ((uint32_t) d.second());
}

/* Clear strip data */
void clear_strip() {
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);
  }
  strip.show();
}

/* Logging. */
void log_time(uint8_t hr, uint8_t min, uint8_t sec) {
  Serial.print(hr, DEC);
  Serial.print(':');
  Serial.print(min, DEC);
  Serial.print(':');
  Serial.print(sec, DEC);
  Serial.print(",  ");
}
void log_times(uint32_t now, uint32_t alarm, uint32_t start) {
  Serial.print(", ");
  Serial.print(now);
  Serial.print(", ");
  Serial.print(alarm);
  Serial.print(", ");
  Serial.print(alarm - now);
  Serial.print(", ");
  Serial.print(start);
  Serial.println();
}

/* Turns on the next fourth LED during start progression. */
void next_LED_on() {
  uint8_t pixel_i = (num_LEDS_on * 4 + num_group_on) % 32;
  strip.setPixelColor(pixel_i, WHITE_COLOR);
  strip.show();
  num_LEDS_on++;
  if (num_LEDS_on % 8 == 0) {
    num_group_on++;
  }
}

/* An "ordered dither" fills every pixel in a sequence that looks
 * sparkly and almost random, but actually follows a specific order. */
void dither(uint32_t c, uint8_t wait) {

  // Determine highest bit needed to represent pixel index
  int hiBit = 0;
  int n = strip.numPixels() - 1;
  for(int bit=1; bit < 0x8000; bit <<= 1) {
    if(n & bit) hiBit = bit;
  }

  int bit, reverse;
  for(int i=0; i<(hiBit << 1); i++) {
    // Reverse the bits in i to create ordered dither:
    reverse = 0;
    for(bit=1; bit <= hiBit; bit <<= 1) {
      reverse <<= 1;
      if(i & bit) reverse |= 1;
    }
    strip.setPixelColor(reverse, c);
    strip.show();
    delay(wait);
  }
  delay(250); // Hold image for 1/4 sec
}

/* Randomly picks a color and speed to dither. */
void random_dither() {
  color cur_color = (color) random(num_colors - 1);
  uint8_t dither_duration = random(1, 26);
  uint8_t dither_r = random(128),
          dither_g = random(128),
          dither_b = random(128);
  uint32_t dither_rand_color = strip.Color(dither_r,dither_g,dither_b);

  switch (cur_color) {
    case WHITE:
      dither(WHITE_COLOR, dither_duration);
      break;
    case RED:
      dither(RED_COLOR, dither_duration);
      break;
    case ORANGE:
      dither(ORANGE_COLOR, dither_duration);
      break;
    case YELLOW:
      dither(YELLOW_COLOR, dither_duration);
      break;
    case MAGENTA:
      dither(MAGENTA_COLOR, dither_duration);
      break;
    case GREEN:
      dither(GREEN_COLOR, dither_duration);
      break;
    case CYAN:
      dither(CYAN_COLOR, dither_duration);
      break;
    case BLUE:
      dither(BLUE_COLOR, dither_duration);
      break;
    case RAND:
      dither(dither_rand_color, dither_duration);
      break;
    default:
      dither(BLUE_COLOR, dither_duration);
      break;
  }

  dither(BLACK_COLOR, 10);
}
