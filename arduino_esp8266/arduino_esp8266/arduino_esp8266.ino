
#define MISO_pin 12
#define MOSI_pin 13
#define SCLK_pin 14
#define CS_pin 5

void release_control() {
  // Surrender the SPI pins
  pinMode(MISO_pin, INPUT); // MISO
  pinMode(MOSI_pin, INPUT); // MOSI
  pinMode(SCLK_pin, INPUT); // SCLK
  pinMode(CS_pin, INPUT); // CS
  // Turn the LED off
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Allow the Pokitto to use the RAM chip
  release_control();
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
