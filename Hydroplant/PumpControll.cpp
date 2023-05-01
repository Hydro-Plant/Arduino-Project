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
  motor_pwm = 0;
}

void PumpControll::setPID(double p, double Ti, double Td) {
  this->p = p * pid_res;
  this->Ti = Ti * pid_res;
  this->Td = Td * pid_res;
}

void PumpControll::setFlow(double wanted_flow) {
  this->wanted_flow = wanted_flow * 1000;
}

double PumpControll::getFlow() {
  return measurement / 1000.0;
}

void PumpControll::update() {
  static unsigned long lastUpdate = 0;
  static unsigned long lastPID = 0;

  if (std_hofmann::overflowUnsignedLong(micros(), lastUpdate + update_rate, update_rate)) {
    unsigned long cur = micros();
    unsigned long long pulseLen = lastPulseLength;
    unsigned long delta_time = cur - lastUpdate;

    if (cur - lastPulses > max_intervall * pulse_measuring_count / 1000) {
      measurement = 0;
      new_measurement = true;
    }else {
      measurement =  (unsigned long long)1000000000000 / (pulseLen * pulse_to_flow);       // (unsigned long long)1000000000 * pulse_to_flow / (pulseLen * 1000000)
                                                                                           // 1000000000: MHz -> ns = 60mHz
    }

    lastUpdate = cur;

    if (this->wanted_flow < min_sensor_flow) {
      motor_pwm = 0;
    }

    if (new_measurement) {
      pid_measurements[2] = pid_measurements[1];
      pid_measurements[1] = pid_measurements[0];
      pid_measurements[0] = wanted_flow - measurement;

      long P = (long long) (pid_measurements[0] - pid_measurements[1]) * this->p / pid_res;
      long I = (long long)(pid_measurements[0] + pid_measurements[1]) * delta_time * pid_res / (2 * this->Ti);
      long D = (long long) ((pid_measurements[0] - pid_measurements[1]) - (pid_measurements[1] - pid_measurements[2])) * this->Td / (delta_time * pid_res);

      motor_pwm += P + I + D;
      motor_pwm = std_hofmann::deckel(motor_pwm, 0, max_pwm);


      if ((motor_pwm != 0) != this->pump_on) {
        this->pump_on = (motor_pwm != 0);
        Serial.print("!SPUMP:" + String(this->pump_on) + "\n");
      }

      new_measurement = false;
      lastPID = cur;
    }
    analogWrite(pump_pin, map(motor_pwm, 0, max_pwm, 0, 255));
  }
}

void PumpControll::flowMeasure() {
  unsigned long cur = micros();
  if (anchor->pulse_counter == anchor->pulse_measuring_count) {
    anchor->lastPulseLength = (cur - anchor->lastPulses) * 1000 / anchor->pulse_measuring_count;  // ns
    anchor->lastPulses = cur;
    anchor->new_measurement = true;
    anchor->pulse_counter = 0;
  }
  anchor->pulse_counter++;
}