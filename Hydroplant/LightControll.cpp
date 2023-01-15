#include "LightControll.h"

LightControll::LightControll(int ldr, int led)
  : ldr_pin(ldr), led_pin(led) {
}

void LightControll::setup(unsigned long daytime, unsigned long timeperday, unsigned long intv, int light_threshold) {
  this->daytime = daytime;
  this->timeperday = timeperday;
  this->intv = intv;
  this->last_update = millis();
  this->light_threshold = light_threshold;
  pinMode(ldr_pin, INPUT);
  pinMode(led_pin, OUTPUT);
}

void LightControll::setTime(unsigned long timeperday) {
  this->timeperday = timeperday;
}

void LightControll::update() {
  if (std_hofmann::overflowUnsignedLong(millis(), last_update + intv, intv)) {
    enum state { turn_off, start_reading,
                 execute };
    static state status = turn_off;

    switch (status) {
      case turn_off:
        digitalWrite(led_pin, LOW);
        turn_off_timer = micros();
        status = start_reading;
        break;
      case start_reading:
        if (analogReader::resultWasRead() && std_hofmann::overflowUnsignedLong(micros(), turn_off_timer + light_turn_off_delay, light_turn_off_delay)) {
          analogReader::startMeasurement(ldr_pin);
          status = execute;
        }
        break;
      case execute:
        if (analogReader::resultAvailable()) {
          int val = analogReader::getResult();
          bool change = false;
          if (val <= light_threshold) {
            digitalWrite(led_pin, LOW);
            if (light_status) change = true;
            light_status = false;
            light_counter++;
          } else {
            if ((long) (daytime / intv - day_counter) <= (long) (timeperday / intv - light_counter)) {
              if (!light_status) change = true;
              light_status = true;
              digitalWrite(led_pin, HIGH);
              light_counter++;
            } else {
              if (light_status) change = true;
              light_status = false;
              digitalWrite(led_pin, LOW);
            }
          }

          if (change) {
            Serial.print("!SLIGT:" + String(light_status) + "\n");
          }

          day_counter++;
          if (day_counter >= daytime / intv) {
            light_per_day = (double)(light_counter * intv) / daytime;
            day_counter = 0;
            light_counter = 0;
          }
          last_update += intv;
          status = turn_off;
        }
        break;
    }
  }
}

double LightControll::getValue() {
  return light_per_day;
}

bool LightControll::getStatus() {
  return light_status;
}