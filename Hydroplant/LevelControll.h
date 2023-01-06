#include <Arduino.h>

#ifndef LEVELCONTROLL_H
#define LEVELCONTROLL_H

class LevelControll {
  private:
    const static unsigned long measure_timeout = 1000000;
    const static double liter_per_mm = 0.2;
    const static long bottom = 70;
    const static long max_sonic = 5000;
    const static long min_sonic = 10;

    int echo_pin;
    int trig_pin;

    unsigned long trigger_start = 0;
    unsigned long pulse_start = 0;
    bool resultsAv = true;
    bool result_compromised = false;
    double result = 0;

    static void echo();
    static LevelControll* anchor;
  public:
    LevelControll(int echo_pin, int trig_pin);
    void setup();
    void startMeasurement();
    bool resultAvailable();
    double getResult();               // -1 if not valid, [mm]
};

#endif LEVELCONTROLL_H