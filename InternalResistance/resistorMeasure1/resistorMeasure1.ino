float readingToVoltage( int );
void printReading( float, float );
float getX( float, float );
float getY( float, float );
boolean switchOnResistorCircuit( void );
boolean switchOffResistorCircuit( void );

float R = 220.0; //Ohm
float v0 = 5.0; //V

int resistorSwitchPin = 8;
int batterySwitchPin = 12;

boolean resistorCircuitOn = false;
boolean batteryCircuitOn = false;

void setup() {
  pinMode( resistorSwitchPin, OUTPUT );
  digitalWrite( resistorSwitchPin, LOW );
  pinMode( batterySwitchPin, OUTPUT );
  digitalWrite( batterySwitchPin, LOW );
  Serial.begin( 9600 );
}

void loop() {
  while (true) {
    resistorCircuitOn = switchOnResistorCircuit();
    float vH = readingToVoltage( analogRead( A0 ) );
    float vL = readingToVoltage( analogRead( A1 ) );
    printReading( vH, vL );
    delay( 100 );
    resistorCircuitOn = switchOffResistorCircuit();
    delay( 500 );
    
  }
}

boolean switchOnResistorCircuit( void ) {
  if (batteryCircuitOn)
    return false;
  else {
    digitalWrite( resistorSwitchPin, HIGH );
    return true;
  }
}

boolean switchOffResistorCircuit( void ) {
  digitalWrite( resistorSwitchPin, LOW );
  return false;
}

float readingToVoltage( int reading ) {
  return (float) reading / 1023.0 * 5.0;
}

float getX( float vH, float vL ) {
  return R * ( vH - vL ) / ( v0 - vH );
}

float getY( float vH, float vL ) {
  return R * vL / ( v0 - vH );
}
void printReading( float vH, float vL ) {
    //Serial.print(  "V_high = " );
    //Serial.print( vH, 4 );
    //Serial.print( ",  V_low = " );
    //Serial.println( vL, 4 );
    //Serial.print( "Opto BJT resistance = " );
    //Serial.println( getX( vH, vL ) );
    Serial.print( "Test resistance = " );
    Serial.println( getY( vH, vL ) );
}
