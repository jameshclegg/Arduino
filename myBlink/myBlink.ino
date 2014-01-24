/*
First sketch just to test out the arduino.
Based on flashing LED sketch from the arduino examples.

James Clegg. 24.01.13.
*/

int onBoardLED = 13;
int onTime = 5
0; //ms
int blinkPeriod = 500; //ms

void setup() {
  pinMode( onBoardLED, OUTPUT );
}

void loop() {
  digitalWrite( onBoardLED, HIGH );
  delay( onTime );
  digitalWrite( onBoardLED, LOW );
  delay( blinkPeriod - onTime );
}
