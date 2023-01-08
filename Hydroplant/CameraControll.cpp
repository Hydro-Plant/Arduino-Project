#include "CameraControll.h"

CameraControll::CameraControll(){

};

void CameraControll::setup(int servo_pin, int dir_pin, int step_pin, int btn_pin) {
  servo.attach(servo_pin);
  stepper.begin(dir_pin, step_pin);
  pinMode(btn_pin, INPUT);
  this->btn_pin = btn_pin;
  pos = 0;
  angle = servo_zero;
  to_pos = 0;
  to_angle = servo_zero;
  bool demo = false;
  cur_state = resetting;
}

void CameraControll::reset() {
  if (cur_state != moving) {
    cur_state = resetting;
  }
}

void CameraControll::goTo(double pos, double angle) {
  this->to_angle = servo_zero + (angle * (double)servo_per_ninety_deg / 90);
  this->to_pos = max_pos * pos;
  this->from_angle = this->angle;
  this->from_pos = this->pos;
  if (!this->firstReset) cur_state = moving;
  this->angle = this->to_angle;
  servo.write(this->angle);
}

void CameraControll::setDemo(bool demo) {
  this->demo = demo;
}

void CameraControll::update() {
  static long last_update = micros();
  if (last_update > micros()) {
    last_update = micros();
  }
  long diff = micros() - last_update;

  static long last_step = micros();
  if (last_step > micros()) {
    last_step = 0;
  }

  bool change = last_state != cur_state;
  switch (cur_state) {
    case waiting:
      // speed = 0;
      break;
    case resetting:
      if (!demo) {
        if (change) {
          stepper.setDirection(backward);
          angle = servo_zero;
          servo.write(angle);
        }
        if (digitalRead(btn_pin)) {
          pos = 0;
          cur_state = waiting;
          if (this->firstReset) {
            this->firstReset = false;
            cur_state = moving;
          }
        } else if (last_step + (1000000 / (steps_per_mm * max_speed)) < micros()) {
          for (int x = 0; x < step_per_update; x++) {
            stepper.step();
          }
          pos--;
          last_step = micros();
        }

      } else {
        cur_state = waiting;
        std_hofmann::debug("Demo: Reset Camera Position");
        pos = 0;
        angle = servo_zero;
      }
      break;
    case moving:
      if (!demo) {
        if (pos == to_pos) {
          servo.write(to_angle);
          cur_state = waiting;
          Serial.print("COK\n");
          break;
        } else if (last_step + (1000000 / (steps_per_mm * max_speed)) < micros()) {
          if (pos > to_pos) {
            stepper.setDirection(backward);
            for (int x = 0; x < step_per_update; x++) {
              stepper.step();
            }
            pos--;
            last_step = micros();
          } else {
            stepper.setDirection(forward);
            for (int x = 0; x < step_per_update; x++) {
              stepper.step();
            }
            pos++;
            last_step = micros();
          }
          /*
          long newangle = abs(pos - from_pos) * (to_angle - from_angle) / abs(to_pos - from_pos) + from_angle;
          if(angle != newangle) {
            angle = newangle;
            servo.write(angle);
          }
          */
        }
      } else {
        pos = to_pos;
        angle = to_angle;
        std_hofmann::debug("Demo: Setting Camera Position");
        Serial.print("COK\n");
        cur_state = waiting;
      }
      break;
  }

  last_state = cur_state;
}