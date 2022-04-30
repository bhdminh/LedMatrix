 /*
  * To read analog value at pin A0 and display the value on P10 panel
  * For connection diagram visit https://goo.gl/HiCJC1
  */
 #include <TimerOne.h>
#include "SPI.h"
#include <ledP10.h>
  
LedP10 myled;

void setup() 
{
  // A- 3, B- 4, STORE- 8, OE-9, 1x Panel
  myled.init(3,4,8,9,1);
}

void loop() 
{
  int sensorValue = analogRead(A0);
  myled.showmsg_single_static(sensorValue,1);
  delay(500);
}
