#include <Servo.h>
#include <Arduino.h>
#include "std_hofmann.h"

#ifndef CAMERACONTROLL_H
#define CAMERACONTROLL_H

class CameraControll {
  Servo rotation;
  double pos;
  double to_pos;
  double angle;
  double to_angle;
  bool demo;
  enum state { waiting,
               resetting,
               moving };
  state cur_state = waiting;

public:

  CameraControll();
  void setup(int servo_pin);
  void reset();
  void goTo(double pos, double angle);
  void setDemo(bool demo);
  void update();
};

#endif CAMERACONTROLL_H