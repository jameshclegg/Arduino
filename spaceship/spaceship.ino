/*
Based on Arduino projects book project 2, spaceship interface.

James Clegg. 24.1.14
*/

int switchState = 0;
int sw = 2;
int gn = 3;
int r0 = 4;
int r1 = 5;
int tDel = 125; //ms

void setup() {
  pinMode( sw, INPUT );
  pinMode( gn, OUTPUT );
  pinMode( r0, OUTPUT );
  pinMode( r1, OUTPUT );
}

void loop() {
  switchState = digitalRead( 2 );
  
  if ( switchState == LOW ) {
   digitalWrite( gn, HIGH );
   digitalWrite( r0, LOW );
   digitalWrite( r1, LOW ); 
  }
  else {
   digitalWrite( gn, LOW );
   digitalWrite( r0, HIGH );
   digitalWrite( r1, LOW );
  
   delay( tDel );
   digitalWrite( r0, LOW );
   digitalWrite( r1, HIGH );
   delay( tDel );
  }
}
