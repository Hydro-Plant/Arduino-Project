#include <Arduino.h>

#ifndef STD_HOFMANN_H
#define STD_HOFMANN_H

class std_hofmann {
public:
  static void debug(String msg);
  static boolean overflowUnsignedLong(unsigned long value, unsigned long threshold);
  static boolean overflowUnsignedLong(unsigned long value, unsigned long threshold, unsigned long timer_diff);
  static long deckel(long value, long min, long max);
  static long deckel(double value, double min, double max);
};

#endif STD_HOFMANN_H