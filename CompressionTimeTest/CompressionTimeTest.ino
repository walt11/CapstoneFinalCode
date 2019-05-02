// Class:       ECE 4970W - Electrical and Computer Engineering Capstone
// Project:     An auto-stabilizing knee brace for physcial therapy and disability applications
// Written by:  John Walter

// This code times how long it takes for the brace to reach maximum compression (servos rotate 180 degrees)
// To test this, a switch connected to digital pin 8 on the Arduino was fixed at the max compression angle
// of the servo. So, when the servo actuates 180 degrees, it toggles the switch ON which breaks the loop and
// stops the timmer.

#include<Servo.h>
Servo serv1, serv2;
void setup() {
  // switch pin 8
  pinMode(8,INPUT);
  Serial.begin(9600);
  // servo pin 5 - Note: servo1 rotates from position 180 degrees (min compression) to position 0 degrees (max compression)
  serv1.attach(5);
  serv1.write(180);
}
unsigned long e;
void loop() {
  unsigned long s = micros();   // get starting time in microseconds
  serv1.write(0);               // tell servo to rotate 180 degrees (max compression)
  // while switch not high, wait
  while(1){
    // when switch is triggered high
    if(digitalRead(8) == HIGH){
      e = micros();             // get ending time in microseconds
      break;
    }
  }
  // calculate the time difference in microseconds
  unsigned long diff = e-s;
  // display time duration
  Serial.print("Time: ");
  Serial.println(diff);
  // position servo back to starting position
  serv1.write(180);
  // retest in 5 seconds
  delay(5000);
}
