#include <DRV8825.h>
#include <Servo.h>
#include <Arduino.h>
#include "std_hofmann.h"

#ifndef CAMERACONTROLL_H
#define CAMERACONTROLL_H

class CameraControll {
  const int microsteping = 1;
  const int servo_zero = 180 / 2;
  const int servo_per_ninety_deg = 180 / 2;
  const int steps_per_mm = 400 * microsteping;
  const int max_pos = 50 * steps_per_mm;
  const int max_speed = 2000;
  const double max_acc = 2000;

  const uint8_t forward = DRV8825_CLOCK_WISE;
  const uint8_t backward = DRV8825_COUNTERCLOCK_WISE;

  Servo servo;
  DRV8825 stepper;
  int btn_pin = 0;

  // double speed = 0;
  // double wanted_speed = 0;

  long pos;
  long to_pos;
  long from_pos;
  long angle;
  long to_angle;
  long from_angle;
  bool demo;
  enum state { waiting,
               resetting,
               moving };
  state cur_state = waiting;
  state last_state = waiting;

public:

  CameraControll();
  void setup(int servo_pin, int dir_pin, int step_pin, int btn_pin);
  void reset();
  void goTo(double pos, double angle);
  void setDemo(bool demo);
  void update();
};

#endif CAMERACONTROLL_H