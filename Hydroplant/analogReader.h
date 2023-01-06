#include <Arduino.h>

#ifndef ANALOGREADER_H
#define ANALOGREADER_H

class analogReader {
  private:
    static bool wasRead;
  public:
    static void setup();
    static void startMeasurement(int pin);
    static bool resultAvailable();
    static bool resultWasRead();
    static int getResult();
};

#endif ANALOGREADER_H