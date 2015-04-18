
int aReadMax = pow(2,10) - 1;
float vMax = 5.0;
int nDPs = 3;

void setup() {
  Serial.begin(9600);
  //Serial.println( aReadMax, DEC );
  Serial.println( "Setup complete" );
}

void loop() {
  long int ctr = 0;
  while (true) {
    int sensorValue = analogRead(A0);
    Serial.print( "Measurement " );
    Serial.print( ++ctr, DEC );
    Serial.print( ": " );
    Serial.print( (float) sensorValue / aReadMax * vMax, nDPs );
    Serial.print( " +/- " );
    Serial.println( vMax/aReadMax, nDPs );
    delay( 2000 );
  }
}
