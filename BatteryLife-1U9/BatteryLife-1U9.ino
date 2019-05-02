// Class:       ECE 4970W - Electrical and Computer Engineering Capstone
// Project:     An auto-stabilizing knee brace for physcial therapy and disability applications
// Written by:  Alex Flynn

// This script samples the battery level, determines wihin what range the level falls, and displays
// that information to the user via an lcd screen.

#include <LiquidCrystal.h>

// battery test pin
int Pin = A0;
// configure LCD display
LiquidCrystal lcd(8,9,10,11,12,13);

void setup() {
  // begin lcd display
  lcd.begin(16,2);
  // configure pin as input
  pinMode(Pin,INPUT);
  Serial.begin(9600);
}

void loop() {
   // sample battery voltage
   float data = analogRead(Pin);
   // max value
   float maxv = 4.8;
   // nominal value
   float nom = 3.7;
   // battery level fraction
   float lev = nom/maxv*100;
   // convert measured value to volts
   float conv = (data/maxv*1023);
   float volts = conv*100;
   Serial.println(data); 
   // if full battery
   if (volts == 100)
   {
     lcd.setCursor(4,0);
     lcd.print("FULL BATTERY");
     delay(1000);
     lcd.clear();
   }
   // if nominal level
   else if (volts < 100 && volts >= lev )
   {
     lcd.setCursor(3,0);
     lcd.print("Nominal Level");
     delay(1000);
     lcd.clear();
   }
   // if low battery
   else if (volts < lev)
   {
     lcd.setCursor(2,0);
     lcd.print("RECHARGE SOON!");
     delay(1000);
     lcd.clear();
   }
   // if no battery connected
   else
   {
    lcd.setCursor(1,0);
    lcd.print("NO BATTERY!");
    delay(1000);
    lcd.clear();
   }
}
