#include <Arduino.h>
#include <Wire.h>
#include "std_hofmann.h"
#include "analogReader.h"

#ifndef LIGHTCONTROLL_H
#define LIGHTCONTROLL_H

class LightControll {
  private:
    const unsigned long light_turn_off_delay = 50;
    const unsigned long measurement_delay = 120000;
    
    int led_pin;
    unsigned long daytime, timeperday, intv;
    
    unsigned long last_update = 0;
    unsigned long day_counter = 0;
    unsigned long light_counter = 0;

    int light_threshold = 0;

    double light_per_day = 0;
    bool light_status = false;
  public:
    LightControll(int led);
    void setup(unsigned long daytime, unsigned long timeperday, unsigned long intv, int light_threshold);
    void setTime(unsigned long timeperday);
    void update();
    double getValue();
    bool getStatus();
};

#endif LIGHTCONTROLL_H