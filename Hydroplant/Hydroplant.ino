void setup() {
  Serial.begin(9600);
}

void loop() {
  if(Serial.available()) {
    String inp = Serial.readString();
    inp.trim();

    if(inp == "?") {
      Serial.println("OK");
    }
  }
}
