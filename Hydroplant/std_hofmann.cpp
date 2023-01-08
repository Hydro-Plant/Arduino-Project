#include "std_hofmann.h"

static void std_hofmann::debug(String msg) {
  Serial.print("DBUG" + msg + "\n");
}

static boolean std_hofmann::overflowUnsignedLong(unsigned long value, unsigned long threshold) {
  return threshold - value >= ((unsigned long)-1) / 2;
}

static boolean std_hofmann::overflowUnsignedLong(unsigned long value, unsigned long threshold, unsigned long timer_diff) {
  return threshold - value >= timer_diff;
}

static double std_hofmann::deckel(double value, double min, double max) {
  if(value < min) value = min;
  else if(value > max) value = max;
  return value;
}