
// for SparkFun pro micro
#define PIN_LED 17

// for others
// #define PIN_LED LED_BUILTIN

void setup() {
  pinMode(PIN_LED, OUTPUT);
}

void loop() {
  digitalWrite(PIN_LED, HIGH);
  delay(200);
  digitalWrite(PIN_LED, LOW);
  delay(800);
}
