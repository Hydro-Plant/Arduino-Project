#include "LevelControll.h"

LevelControll* LevelControll::anchor = nullptr;

LevelControll::LevelControll(int echo_pin, int trig_pin) {
  this->echo_pin = echo_pin;
  this->trig_pin = trig_pin;
  this->anchor = this;
}

void LevelControll::setup() {
  pinMode(trig_pin, OUTPUT);
  pinMode(echo_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(echo_pin), LevelControll::echo, CHANGE);
}

void LevelControll::startMeasurement() {
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);
  resultsAv = false;
  result_compromised = false;
  measuring_started = false;
  trigger_start = millis();
}

bool LevelControll::resultAvailable() {
  if (millis() - trigger_start > 10000 && !measuring_started) {
    result_compromised = true;
    resultsAv = true;
    result = -1;
  }
  return resultsAv;
}

double LevelControll::getResult() {
  return this->result;
}

double LevelControll::getMM() {
  return this->mm;
}

static void LevelControll::echo() {
  if (digitalRead(anchor->echo_pin)) {
    anchor->pulse_start = micros();
    anchor->measuring_started = true;
  } else {
    unsigned long cur = micros();
    if (!anchor->result_compromised) {
      long mm = (long)((cur - anchor->pulse_start) / 2.91 / 2);
      anchor->mm = mm;
      if (mm > max_sonic || mm < min_sonic) {
        anchor->result = -1;
      } else {
        anchor->result = (bottom - mm) * liter_per_mm;
      }
      anchor->resultsAv = true;
    }
  }
}