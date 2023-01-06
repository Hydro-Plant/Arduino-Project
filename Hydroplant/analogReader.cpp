#include "analogReader.h"

static bool analogReader::wasRead = true;

static void analogReader::setup() {
  ADCSRA = bit(ADEN)                                // Turn ADC on
           | bit(ADPS0) | bit(ADPS1) | bit(ADPS2);  // Prescaler of 128
}

static void analogReader::startMeasurement(int pin) {
  if (bit_is_clear(ADCSRA, ADSC)) {
    wasRead = false;
    ADMUX = bit(REFS0)              // AVCC
            | ((pin - 14) & 0x07);  // Arduino Uno to ADC pin
    bitSet(ADCSRA, ADSC);           // Start a conversion
  }
}

static bool analogReader::resultAvailable() {
  return bit_is_clear(ADCSRA, ADSC);
}

static bool analogReader::resultWasRead() {
  return analogReader::wasRead;
}

static int analogReader::getResult() {
  wasRead = true;
  return ADC;
}