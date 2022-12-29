#include "CameraControll.h"
#include "std_hofmann.h"

#define SERVO 3
#define STEP 5
#define DIR 4
#define BTN 2

const bool demo = false;

bool execute_command = false;
String com_buffer = "";

int value_intervall = 10000;
long mlls = 0;

double temp_val = 0;
double light_val = 0;
bool light_status = false;
double ph_val = 0;
double ec_val = 0;
double flow_val = 0;
double level_val = 0;

CameraControll cc;

void reset() {
  mlls = 0;
}

void setup() {
  Serial.begin(115200);
  mlls = millis();
  randomSeed(analogRead(0));
  cc.setup(SERVO, DIR, STEP, BTN);
  cc.setDemo(demo);

  if (demo) {
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(A4, INPUT);
  }
}

void loop() {
  if (Serial.available() > 0) {  // Handle inputs
    char x = Serial.read();
    if (x != '\n') {
      com_buffer += x;
    } else {
      execute_command = true;
    }
  }
  if (execute_command) {
    com_buffer.trim();
    if (com_buffer == "?") {
      Serial.write("OK");
    } else if (com_buffer.length() >= 5 && com_buffer.substring(0, 1) == "O") {
      if (com_buffer.substring(1, 5) == "INTV") {
        value_intervall = com_buffer.substring(5, com_buffer.length()).toInt();
      }
    } else if (com_buffer.length() >= 2 && com_buffer.substring(0, 2) == "CR") {
      cc.reset();
    } else if (com_buffer.length() >= 1 && com_buffer.substring(0, 1) == "C") {
      int pos = com_buffer.indexOf(";");
      double new_pos = com_buffer.substring(1, pos).toDouble();
      double new_angle = com_buffer.substring(pos + 1, com_buffer.length()).toDouble();
      cc.goTo(new_pos, -new_angle);
    } else {
      std_hofmann::debug("FALSE");
    }
    std_hofmann::debug(com_buffer);
    com_buffer = "";
    execute_command = false;
  }

  if (mlls + value_intervall <= millis()) {
    mlls = millis();
    if (demo) {

      temp_val = (double)map(analogRead(A0), 0, 1023, 14000, 27000) / 1000;
      light_val = (double)map(analogRead(A1), 0, 1023, 0000, 24000) / 1000;
      light_status = sin(millis() / 10000) >= 0;
      ph_val = (double)map(analogRead(A2), 0, 1023, 0000, 14000) / 1000;
      ec_val = (double)random(0, 5000) / 1000;
      flow_val = (double)map(analogRead(A3), 0, 1023, 0000, 10000) / 1000;
      level_val = (double)map(analogRead(A4), 0, 1023, 0000, 10000) / 1000;

      /*
      temp_val = (double)map(random(0, 1024), 0, 1023, 14000, 27000) / 1000;
      light_val = (double)map(random(0, 1024), 0, 1023, 0000, 24000) / 1000;
      light_status = sin(millis() / 10000) >= 0;
      ph_val = (double)map(random(0, 1024), 0, 1023, 0000, 14000) / 1000;
      ec_val = (double)random(0, 5000) / 1000;
      flow_val = (double)map(random(0, 1024), 0, 1023, 0000, 10000) / 1000;
      level_val = (double)map(random(0, 1024), 0, 1023, 0000, 10000) / 1000;
      */
    }

    Serial.print("VTEMP" + String(temp_val) + "\n");
    Serial.print("VLIGT" + String(light_val) + "\n");
    Serial.print("SLIGT" + String(light_status) + "\n");
    Serial.print("VPH" + String(ph_val) + "\n");
    Serial.print("VEC" + String(ec_val) + "\n");
    Serial.print("VFLOW" + String(flow_val) + "\n");
    Serial.print("VLEVL" + String(level_val) + "\n");
  }

  cc.update();
}