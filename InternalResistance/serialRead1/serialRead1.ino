
// top portion of code turns on and off an LED with keyboard input from the user.
// this will be used to only read analogue voltages on key press.
// bottom code reads analogue voltages and prints to serial
//
//
// James Clegg. 18th April 2015.

int delayTimeMs = 10;
void setup() {
        Serial.begin(9600);     // opens serial port, sets data rate to 9600 bps
        pinMode(13,OUTPUT);
        digitalWrite(13, LOW);
}

void loop() {
        int ii = 0, wLen = 0;
        char nextByte, thisWord[ 10 ];
        // send data only when you receive data:
        while (Serial.available() > 0 && wLen <= 10 ) {
                // read the incoming byte:
                nextByte = Serial.read();
                thisWord[wLen] = nextByte;
                delay( delayTimeMs );
                ++wLen;
        }
        while (Serial.available() > 0) {
          Serial.read();
          delay( delayTimeMs );
        }
        // say what you got:
        if (wLen>0) {
        Serial.print("I received: ");
        for (int jj = 0; jj < wLen; ++jj ){
            Serial.print( thisWord[jj] );
        }
        Serial.println( thisWord[ wLen ] );
        }
       

        if (wLen == 1 && (thisWord[0] == 'h' || thisWord[0] == 'H') )
          digitalWrite(13, HIGH);
        else if (wLen > 0)
          digitalWrite(13, LOW);
        
}

/*
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
*/
