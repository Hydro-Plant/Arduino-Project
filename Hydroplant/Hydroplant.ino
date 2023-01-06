#include "CameraControll.h"
#include "LightControll.h"
#include "PumpControll.h"
#include "LevelControll.h"
#include "analogReader.h"
#include "std_hofmann.h"

#define SERVO 9
#define STEP 5
#define DIR 4
#define BTN 7
#define LDR A0
#define LED 6
#define PUMP 11
#define FLOW 2
#define ECHO 3
#define TRIG 8
#define TEMP A1
#define PH A2
#define EC A3

const bool demo = false;

bool execute_command = false;
String com_buffer = "";

int value_intervall = 5000;
unsigned long last_measurement = 0;

CameraControll cc;
LightControll lc(LDR, LED);
PumpControll pc(PUMP, FLOW);
LevelControll lvlc(ECHO, TRIG);

void setup() {
  Serial.begin(115200);
  last_measurement = millis();

  cc.setup(SERVO, DIR, STEP, BTN);

  // lc.setup(86400000, 54000000, 300000, 1024 / 2);
  lc.setup(20000, 0, 1000, 360);

  pc.setup();
  pc.setPID(0.1, 0.01, 0.00001);
  pc.setFlow(0);

  lvlc.setup();

  analogReader::setup();

  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
}

void loop() {
  if (Serial.available() > 0) {  // Handle inputs
    char x = Serial.read();
    if (x == '?') {
      delay(1000);
      Serial.print("OK");
    } else if (x != '\n') {
      com_buffer += x;
    } else {
      execute_command = true;
    }
  }

  if (execute_command) {
    com_buffer.trim();
    while (!isAlpha(com_buffer[0])) {
      com_buffer.remove(0, 1);
    }
    if (com_buffer.length() >= 5 && com_buffer.substring(0, 1) == "O") {
      String option = com_buffer.substring(1, 5);
      if (option == "INTV") {
        value_intervall = com_buffer.substring(5, com_buffer.length()).toInt();
      } else if (option == "LIGT") {
        unsigned long time = (unsigned long)(com_buffer.substring(5, com_buffer.length()).toDouble());  // * 86400000);
        std_hofmann::debug("Light-time: " + String(time));
        lc.setTime(time);
      }
    } else if (com_buffer.length() >= 2 && com_buffer.substring(0, 2) == "CR") {
      cc.reset();
    } else if (com_buffer.length() >= 1 && com_buffer.substring(0, 1) == "C") {
      int pos = com_buffer.indexOf(";");
      double new_pos = com_buffer.substring(1, pos).toDouble();
      double new_angle = com_buffer.substring(pos + 1, com_buffer.length()).toDouble();
      cc.goTo(new_pos, -new_angle);
      std_hofmann::debug("Camera: " + String(new_pos, 2) + " " + String(new_angle, 2));
    } else {
      String hex = "";
      for (int x = 0; x < com_buffer.length(); x++) {
        int character = com_buffer[x];
        hex += String(character) + " ";
      }
      std_hofmann::debug("NOTHING: " + hex);
    }
    com_buffer = "";
    execute_command = false;
  }

  if (std_hofmann::overflowUnsignedLong(millis(), last_measurement + value_intervall, value_intervall)) {
    enum measurement_states { start_measurement,
                              send_value,
                              start_temp,
                              send_temp,
                              start_ph,
                              send_ph,
                              start_tds,
                              send_tds,
                              done };

    static measurement_states level_state = start_measurement;  // Level measurement
    switch (level_state) {
      case start_measurement:
        lvlc.startMeasurement();
        level_state = send_value;
        break;
      case send_value:
        if (lvlc.resultAvailable()) {
          double res = lvlc.getResult();
          if (res != -1) {
            Serial.print("VLEVL" + String(res, 1) + "\n");
          }
          level_state = done;
        }
        break;
      case done:
        break;
    }

    static measurement_states light_flow_state = send_value;  // Light & Flow measurement
    switch (light_flow_state) {
      case send_value:
        Serial.print("VLIGT" + String(lc.getValue() * 24) + "\n");
        Serial.print("VFLOW" + String(pc.getFlow(), 2) + "\n");
        light_flow_state = done;
        break;
      case done:
        break;
    }

    static measurement_states temp_ph_ec_state = start_temp;
    static double temperature_value = 0;
    switch (temp_ph_ec_state) {
      case start_temp:
        if (analogReader::resultWasRead()) {
          analogReader::startMeasurement(TEMP);
          temp_ph_ec_state = send_temp;
        }
        break;
      case send_temp:
        if (analogReader::resultAvailable()) {
          int val = analogReader::getResult();
          int res = val;
          temperature_value = res;
          Serial.print("VTEMP" + String(res) + "\n");
          temp_ph_ec_state = start_ph;
        }
        break;
      case start_ph:
        if (analogReader::resultWasRead()) {
          analogReader::startMeasurement(PH);
          temp_ph_ec_state = send_ph;
        }
        break;
      case send_ph:
        if (analogReader::resultAvailable()) {
          double val = analogReader::getResult();
          double res = (-5.6548) * val * 5.0 / 1024.0 + 15.509;
          Serial.print("VPH" + String(res, 5) + "\n");
          temp_ph_ec_state = start_tds;
        }
        break;
      case start_tds:
        if (analogReader::resultWasRead()) {
          analogReader::startMeasurement(EC);
          temp_ph_ec_state = send_tds;
        }
        break;
      case send_tds:
        if (analogReader::resultAvailable()) {
          int val = analogReader::getResult();
          double compensationCoefficient = 1.0 + 0.02 * (temperature_value - 25.0);                                                                                                                //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
          double compensationVolatge = (val * 5.0 / 1024.0) / compensationCoefficient;                                                                                                             //temperature compensation
          double tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5;  //convert voltage value to tds value
          Serial.print("VTDS" + String(tdsValue) + "\n");
          temp_ph_ec_state = done;
        }
        break;
      case done:
        break;
    }

    if (level_state == done && light_flow_state == done && temp_ph_ec_state == done) {
      last_measurement = millis();
      level_state = start_measurement;
      light_flow_state = send_value;
      temp_ph_ec_state = start_temp;
    }
  }

  pc.update();
  cc.update();
  lc.update();
}