#include "analogReader.h"

static bool analogReader::wasRead = true;

static void analogReader::setup() {
  ADCSRA = bit(ADEN)                                // Turn ADC on
           | bit(ADPS0) | bit(ADPS1) | bit(ADPS2);  // Prescaler of 128
}

static void analogReader::startMeasurement(int pin) {
#if defined(__AVR_ATmega32U4__)
  if (pin >= 18) pin -= 18;  // allow for channel or pin numbers
#endif
  pin = analogPinToChannel(pin);
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  if (pin >= 54) pin -= 54;  // allow for channel or pin numbers
#elif defined(__AVR_ATmega32U4__)
  if (pin >= 18) pin -= 18;  // allow for channel or pin numbers
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
  if (pin >= 24) pin -= 24;  // allow for channel or pin numbers
#else
  if (pin >= 14) pin -= 14;  // allow for channel or pin numbers
#endif
  if (bit_is_clear(ADCSRA, ADSC)) {
    wasRead = false;
    ADMUX = bit(REFS0)              // AVCC
            | (pin & 0x07);  // Arduino Uno to ADC pin
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