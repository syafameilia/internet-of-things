#include <Arduino.h>
void setup() {
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
}

void loop() {
  digitalWrite(2, HIGH);
  delay(30000);
  digitalWrite(2, LOW);

  digitalWrite(4, HIGH);
  delay(5000);
  digitalWrite(4, LOW);

  digitalWrite(5, HIGH);
  delay(20000);
  digitalWrite(5, LOW);
}
