#include "PumpControll.h"

PumpControll* PumpControll::anchor = nullptr;

PumpControll::PumpControll(int pump_pin, int flow_pin) {
  this->pump_pin = pump_pin;
  this->flow_pin = flow_pin;
  this->anchor = this;
}

void PumpControll::setup() {
  pinMode(this->pump_pin, OUTPUT);
  pinMode(this->flow_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(flow_pin), PumpControll::flowMeasure, RISING);
  measurement = 0;
}

void PumpControll::setPID(double p, double Ti, double Td) {
  this->p = p;
  this->Ti = Ti;
  this->Td = Td;
}

void PumpControll::setFlow(double wanted_flow) {
  this->wanted_flow = wanted_flow;
}

double PumpControll::getFlow() {
  return measurement;
}

void PumpControll::update() {
  if (std_hofmann::overflowUnsignedLong(micros(), lastUpdate + update_rate, update_rate) && !zero) {
    unsigned long sum = 0;
    for (int x = 0; x < mes_len - 1; x++) {
      sum += measurements[x] - measurements[x + 1];
    }
    double averg = (double)sum / (double)(mes_len - 1);
    measurement = (double)(60000000) / (double)(averg * wave_per_l);

    unsigned long cur = micros();
    double delta_time = (double)(cur - lastUpdate) / 1000000;
    if (micros() - measurements[0] > max_diff) {
      zero = true;
      measurement = 0;
      pid_measurements[0] = pid_measurements[1] = pid_measurements[2] = 0;
      new_measurement = true;
      if (wanted_flow < min_sensor_flow) {
        motor_pwm = 0;
      }
    }

    if (new_measurement) {
      new_measurement = false;
      pid_measurements[2] = pid_measurements[1];
      pid_measurements[1] = pid_measurements[0];
      pid_measurements[0] = wanted_flow - measurement;


      double P = (pid_measurements[0] - pid_measurements[1]) * this->p;
      double I = (pid_measurements[0] + pid_measurements[1]) * delta_time / (2 * this->Ti);
      double D = ((pid_measurements[0] - pid_measurements[1]) - (pid_measurements[1] - pid_measurements[2])) * this->Td / delta_time;

      motor_pwm += P + I + D;
      motor_pwm = std_hofmann::deckel(motor_pwm, 0, 255);

      analogWrite(pump_pin, (int)motor_pwm);
      lastUpdate = cur;
    }
  }
}

void PumpControll::flowMeasure() {
  PumpControll::anchor->zero = false;
  unsigned long cur = micros();
  for (int x = PumpControll::anchor->mes_len - 1; x > 0; x--) {
    PumpControll::anchor->measurements[x] = PumpControll::anchor->measurements[x - 1];
  }
  PumpControll::anchor->measurements[0] = cur;
  PumpControll::anchor->new_measurement = true;
}