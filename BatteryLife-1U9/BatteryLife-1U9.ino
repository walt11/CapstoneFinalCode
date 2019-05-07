// Class:       ECE 4970W - Electrical and Computer Engineering Capstone
// Project:     An auto-stabilizing knee brace for physcial therapy and disability applications
// Written by:  Alex Flynn

// This script samples the battery level, determines wihin what range the level falls, and displays
// that information to the user via an lcd screen.

#include <LiquidCrystal.h>

#include <LiquidCrystal.h>

int Pin = A0;

LiquidCrystal lcd(8,9,10,11,12,13);

// the setup routine runs once when you press reset:
void setup() {
  lcd.begin(16,2);
  pinMode(Pin,INPUT);
  Serial.begin(9600);
  
}

void loop() {
   float data = analogRead(Pin);
   float upper = 4.75;
   float lower = 4.5; 
   float volts = (data/1023*5);
   Serial.println(volts,4); 
   if (volts > upper)
   {
   lcd.setCursor(4,0);
   lcd.print("FULL BATTERY");
   delay(1000);
   lcd.clear();
   }
   else if (volts <= upper && volts >= lower  )
   {
   lcd.setCursor(3,0);
   lcd.print("Nominal Level");
   delay(1000);
   lcd.clear();
   }
   else if (volts < lower && volts >= 2.0)
   {
   lcd.setCursor(2,0);
   lcd.print("RECHARGE SOON!");
   delay(1000);
   lcd.clear();
   }
   else{
    lcd.setCursor(5,0)
    lcd.print("Battery off");
    
   }

}

