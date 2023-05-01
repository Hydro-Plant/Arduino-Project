#include <Wire.h>
#include "CameraControll.h"
#include "LightControll.h"
#include "PumpControll.h"
#include "LevelControll.h"
#include "analogReader.h"
#include "std_hofmann.h"

#define SERVO 9
#define STEP 5
#define DIR 4
#define EN 10
#define BTN 7
#define LDR_SDA A4
#define LDR_SCL A5
#define LED 6
#define PUMP 11
#define FLOW 2
#define ECHO 3
#define TRIG 8
#define TEMP A1       // A1, zurückändern nicht vergessen
#define PH A2         // A2, zurückändern nicht vergessen
#define EC A3
#define BAT A0

const bool demo = false;

bool com_start = false;
bool execute_command = false;
String com_buffer = "";

int value_intervall = 5000;
unsigned long last_measurement = 0;

uint16_t pt_list[] = 
{
  10000, 10039, 10078, 10117, 10156, 10195, 10234, 10273, 10312, 10351,
  10390, 10429, 10468, 10507, 10546, 10850, 10624, 10663, 10702, 10740,
  10779, 10818, 10857, 10896, 10935, 10973, 11012, 11051, 11090, 11129,
  11167, 11206, 11245, 11283, 11322, 11361, 11400, 11438, 11477, 11515,
  11554, 11593, 11631, 11670, 11708, 11747, 11786, 11824, 11863, 11901
};

CameraControll cc;
LightControll lc(LED);
PumpControll pc(PUMP, FLOW);
LevelControll lvlc(ECHO, TRIG);

void setup() {
  Serial.begin(19200);
  Wire.begin();
  last_measurement = millis();

  cc.setup(SERVO, DIR, STEP, EN, BTN);
  cc.reset();

  // lc.setup(86400000, 54000000, 300000, 1024 / 2);
  lc.setup(20000, 0, 1000, 100);

  pc.setup();
  pc.setPID(0, 500, 0);
  pc.setFlow(0);

  lvlc.setup();

  analogReference(EXTERNAL);
  analogReader::setup();

  pinMode(TEMP, INPUT);
  pinMode(PH, INPUT);
  pinMode(EC, INPUT);
  pinMode(BAT, INPUT);
  interrupts();
}

void loop() {
  if (Serial.available() > 0) {  // Handle inputs
    char x = Serial.read();
    switch (x) {
      case '?':
        delay(1000);
        Serial.print("OK");
        break;
      case '\n':
        com_start = false;
        execute_command = true;
        break;
      case '!':
        com_start = true;
      default:
        if (com_start) com_buffer += x;
        break;
    }
  }

  if (execute_command) {
    com_buffer.trim();

    String com = "";
    String sub_com = "";
    String data = "";
    if (com_buffer.length() >= 2) {
      com = com_buffer.substring(1, 2);
      int pos = com_buffer.indexOf(':');
      if (pos != -1) {
        sub_com = com_buffer.substring(2, pos);
        data = com_buffer.substring(pos + 1, com_buffer.length());
      } else {
        sub_com = com_buffer.substring(2, com_buffer.length());
      }

      if (com == "O") {
        if (sub_com == "INTV") {
          value_intervall = data.toInt();
        } else if (sub_com == "LIGT") {
          unsigned long time = (unsigned long)(data.toDouble() * 1000);  // * 86400000);
          std_hofmann::debug("Light-time: " + String(time));
          lc.setTime(time);
        } else if (sub_com == "PUMP") {
          double flow = data.toDouble();
          std_hofmann::debug("Pump Flow: " + String(flow, 1));
          pc.setFlow(flow);
        }
      } else if (com == "C") {
        if (sub_com == "GO") {
          int pos = data.indexOf(';');
          double new_pos = data.substring(0, pos).toDouble();
          double new_angle = data.substring(pos + 1, data.length()).toDouble();
          cc.goTo(new_pos, -new_angle);
          std_hofmann::debug("Camera: " + String(new_pos, 2) + " " + String(new_angle, 2));
        } else if (sub_com == "R") {
          cc.reset();
        }
      } else {
        String hex = "";
        for (int x = 0; x < com_buffer.length(); x++) {
          int character = com_buffer[x];
          hex += String(character) + " ";
        }
        std_hofmann::debug("NOTHING: " + hex);
      }
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
                              start_bat,
                              send_bat,
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
            Serial.print("!VLEVL:" + String(res, 1) + "\n");
          } else {
            std_hofmann::debug("Level-reading failed: " + String(lvlc.getMM(), 4));
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
        Serial.print("!VLIGT:" + String(lc.getValue() * 24) + "\n");
        Serial.print("!VFLOW:" + String(pc.getFlow(), 2) + "\n");
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
          long val = analogReader::getResult();
          unsigned long resistor = (val * (unsigned long long)9960) / (1023 - val);
          if(resistor < pt_list[0]) {
            temperature_value = 0;
          } else if(resistor > pt_list[49]) {
            temperature_value = 49;
          } else {
            for(int x = 0; x < 49; x++) {
              uint16_t lower = pt_list[x];
              uint16_t upper = pt_list[x + 1];
              if(resistor <= upper && resistor >= lower) {
                temperature_value = x + (resistor - lower) / (double)(upper - lower);
                break;
              }
            }
          }
          std_hofmann::debug("T-VAL + RES: " + String(val) + " " + String(resistor));
          Serial.print("!VTEMP:" + String(temperature_value) + "\n");
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
          double res = (-5.6548) * val * 5.0 / 1023.0 + 15.509;
          Serial.print("!VPH:" + String(res, 5) + "\n");
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
          Serial.print("!VTDS:" + String(tdsValue) + "\n");
          temp_ph_ec_state = start_bat;
        }
        break;
      case start_bat:
        if (analogReader::resultWasRead()) {
          analogReader::startMeasurement(BAT);
          temp_ph_ec_state = send_bat;
        }
        break;
      case send_bat:
        if (analogReader::resultAvailable()) {
          int val = analogReader::getResult();
          long batValue = (long)100 * (val - 682) / (1023 - 682);
          Serial.print("!VBAT:" + String(batValue) + "\n");
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