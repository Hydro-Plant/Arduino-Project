#include "CameraControll.h"

CameraControll::CameraControll(){

};

void CameraControll::setup(int servo_pin, int dir_pin, int step_pin, int btn_pin) {
  servo.attach(servo_pin);
  stepper.begin(dir_pin, step_pin);
  pinMode(btn_pin, INPUT);
  this->btn_pin = btn_pin;
  pos = 0;
  angle = 0;
  to_pos = 0;
  to_angle = 0;
  bool demo = false;
}

void CameraControll::reset() {
  if (cur_state != moving) {
    cur_state = resetting;
  }
}

void CameraControll::goTo(double pos, double angle) {
  std_hofmann::debug("I should go to");
  this->to_angle = servo_zero + (angle  * (double)servo_per_ninety_deg / 90);
  this->to_pos = max_pos * pos;
  this->from_angle = this->angle;
  this->from_pos = this->pos;
  cur_state = moving;
}

void CameraControll::setDemo(bool demo) {
  this->demo = demo;
}

void CameraControll::update() {
  static long last_update = micros();
  if(last_update > micros()) {
    last_update = micros();
  }
  long diff = micros() - last_update;

  static long last_step = micros();
  if(last_step > micros()) {
    last_step = 0;
  }

  bool change = last_state != cur_state;
  switch (cur_state) {
    case waiting:
      // speed = 0;
      break;
    case resetting:
      if (!demo) {
        if(change) {
          stepper.setDirection(backward);
          angle = servo_zero;
          servo.write(angle);
        }
        if(digitalRead(btn_pin)) {
          pos = 0;
          cur_state = waiting;
        } else if(last_step + (1000000 / max_speed) < micros()) {
          stepper.step();
          pos--;
          last_step = micros();
        }

      } else {
        std_hofmann::debug("Demo: Reset Camera Position");
        cur_state = waiting;
        pos = 0;
        angle = 0;
        to_pos = 0;
        to_angle = 0;
      }
      break;
    case moving:
      if (!demo) {
        if(pos == to_pos) {
          servo.write(to_angle);
          cur_state = waiting;
          Serial.print("COK\n");
          break;
        } else if(last_step + (1000000 / max_speed) < micros()) {
          if(pos > to_pos) {
            stepper.setDirection(backward);
            stepper.step();
            pos--;
            last_step = micros();
          }else {
            stepper.setDirection(forward);
            stepper.step();
            pos++;
            last_step = micros();
          }
          angle = floor(((float) abs(pos - from_pos) / (float) abs(to_pos - from_pos)) * (to_angle - from_angle) + from_angle);
          servo.write(angle);
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