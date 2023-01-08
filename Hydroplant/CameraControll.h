#include <DRV8825.h>
#include <Servo.h>
#include <Arduino.h>
#include "std_hofmann.h"

#ifndef CAMERACONTROLL_H
#define CAMERACONTROLL_H

class CameraControll {
  const long microsteping = 32;
  const long step_per_update = 1;
  const int servo_zero = 88;
  const int servo_per_ninety_deg = 180 / 2;
  const double steps_per_mm = 200 * microsteping / (0.7 * step_per_update);               // 200 steps/U, 0.7 mm/U
  const long max_pos = 45 * steps_per_mm;
  const double max_speed = 10;                                // mm / s
  const double max_acc = 2000;

  const uint8_t forward = DRV8825_CLOCK_WISE;
  const uint8_t backward = DRV8825_COUNTERCLOCK_WISE;

  Servo servo;
  DRV8825 stepper;
  int btn_pin = 0;

  bool firstReset = true;

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