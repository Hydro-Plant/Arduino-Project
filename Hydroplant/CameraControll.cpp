#include "CameraControll.h"

CameraControll::CameraControll(){

};

void CameraControll::setup(int servo_pin) {
  rotation.attach(servo_pin);
  pos = 0;
  angle = 0;
  to_pos = 0;
  to_angle = 0;
  bool demo = false;
}

void CameraControll::reset() {
  std_hofmann::debug("I should reset");
  if (cur_state != moving) {
    std_hofmann::debug("I should and will reset because " + cur_state);
    cur_state = resetting;
  }
}

void CameraControll::goTo(double pos, double angle) {
  std_hofmann::debug("I should go to");
  this->to_angle = angle;
  this->to_pos = pos;
  cur_state = moving;
}

void CameraControll::setDemo(bool demo) {
  this->demo = demo;
}

void CameraControll::update() {
  switch (cur_state) {
    case waiting:

      break;
    case resetting:
      if (demo) {
        std_hofmann::debug("Demo: Reset Camera Position");
        cur_state = waiting;
        pos = 0;
        angle = 0;
        to_pos = 0;
        to_angle = 0;
      }
      break;
    case moving:
      if (demo) {
        pos = to_pos;
        angle = to_angle;
        std_hofmann::debug("Demo: Setting Camera Position");
        Serial.print("COK\n");
        cur_state = waiting;
      }
      break;
  }
}