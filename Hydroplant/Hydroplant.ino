#include "CameraControll.h"
#include "std_hofmann.h"

bool demo = true;

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
  cc.setup(0);
  cc.setDemo(true);

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
    String inp = Serial.readString();
    inp.trim();
    if (inp == "?") {
      Serial.write("OK");
    } else if (inp.length() >= 2 && inp.substring(0, 2) == "VT") {
      int len = String((char)inp[2]).toInt();
      value_intervall = inp.substring(3, 3 + len).toInt();
    } else if (inp.length() >= 2 && inp.substring(0, 2) == "CR") {
      cc.reset();
    } else if (inp.length() >= 1 && inp.substring(0, 1) == "C") {
      char *c = &inp[0];
      int pos = strchr(c, ';');
      double new_pos = inp.substring(1, pos).toDouble();
      double new_angle = inp.substring(pos + 1, inp.length()).toDouble();
      cc.goTo(new_pos, new_angle);
    } else {
      std_hofmann::debug("FALSE");
    }
  }
  if (mlls + value_intervall <= millis()) {
    mlls = millis();
    if (demo) {
      /*
      temp_val = (double)map(analogRead(A0), 0, 1023, 14000, 27000) / 1000;
      light_val = (double)map(analogRead(A1), 0, 1023, 0000, 24000) / 1000;
      light_status = sin(millis() / 10000) >= 0;
      ph_val = (double)map(analogRead(A2), 0, 1023, 0000, 14000) / 1000;
      ec_val = (double)random(0, 5000) / 1000;
      flow_val = (double)map(analogRead(A3), 0, 1023, 0000, 10000) / 1000;
      level_val = (double)map(analogRead(A4), 0, 1023, 0000, 10000) / 1000;
      */

      temp_val = (double)map(random(0, 1024), 0, 1023, 14000, 27000) / 1000;
      light_val = (double)map(random(0, 1024), 0, 1023, 0000, 24000) / 1000;
      light_status = sin(millis() / 10000) >= 0;
      ph_val = (double)map(random(0, 1024), 0, 1023, 0000, 14000) / 1000;
      ec_val = (double)random(0, 5000) / 1000;
      flow_val = (double)map(random(0, 1024), 0, 1023, 0000, 10000) / 1000;
      level_val = (double)map(random(0, 1024), 0, 1023, 0000, 10000) / 1000;
    }

    Serial.print("VTEMP" + String(temp_val) + "\n");
    Serial.print("VLIGT" + String(light_val) + "\n");
    Serial.print("VLGST" + String(light_status) + "\n");
    Serial.print("VPH" + String(ph_val) + "\n");
    Serial.print("VEC" + String(ec_val) + "\n");
    Serial.print("VFLOW" + String(flow_val) + "\n");
    Serial.print("VLEVL" + String(level_val) + "\n");
  }

  cc.update();
}