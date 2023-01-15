#include <Arduino.h>
#include "std_hofmann.h"

#ifndef PUMPCONTROLL_H
#define PUMPCONTROLL_H

class PumpControll {
  private:
    const double min_sensor_flow = 0.3;    // Smaller = 0; l / min
    const unsigned long wave_per_l = 5880;
    const unsigned long max_diff = 60000000 / (min_sensor_flow * wave_per_l);
    const unsigned long update_rate = 10000;           // us

    int pump_pin;
    int flow_pin;

    double motor_pwm = 0;

    double pid_measurements[3];
    int mes_len = 20;
    unsigned long measurements[20];
    double measurement = 0;
    bool new_measurement = false;
    bool zero = true;
    bool pump_on = false;

    double p = 1;
    double Ti = 1;
    double Td = 1;

    double wanted_flow = 0;

    unsigned long lastUpdate = 0;

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