#include <Arduino.h>
#include "std_hofmann.h"

#ifndef PUMPCONTROLL_H
#define PUMPCONTROLL_H

class PumpControll {
  private:
    const unsigned long min_sensor_flow = 1000;    // Smaller = 0; ml / min
    const unsigned long pulse_to_flow = 11;            // * 10^-6
    const unsigned long max_intervall = (unsigned long long)1000000000000 / (min_sensor_flow * pulse_to_flow);     // 60000000000: 60.000.000 = min -> µs      1000 = ml -> l
    const unsigned long update_rate = 2000;           // us
    const unsigned long pulse_measuring_count = 10;

    const long pid_res = 10000;
    const long pwm_multiplicator = 1000;
    const long max_pwm = 255 * pwm_multiplicator;

    int pump_pin;
    int flow_pin;

    long motor_pwm = 0;

    long pid_measurements[3];
    unsigned long measurement = 0;
    bool new_measurement = false;
    bool pump_on = false;

    long p = 1;
    long Ti = 1;
    long Td = 1;

    unsigned long wanted_flow = 0;                // ml / min

    unsigned long lastPulses = 0;
    unsigned long lastPulseLength = 0;
    unsigned long long pulse_counter = 0;

    static void flowMeasure();
    static PumpControll* anchor;
  public:
    PumpControll(int pump_pin, int flow_pin);
    void setup();
    void setPID(double p, double Ti, double Td);
    void setFlow(double wanted_flow);
    double getFlow();
    void update();
};

#endif PUMPCONTROLL_H