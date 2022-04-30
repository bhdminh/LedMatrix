/*
 * This example is to display a counter value.
 * showmsg_single_static function takes two arguments
 * first argument can be an interger value or a pointer to character string or a string in double quotes.
 * second argument is to select font, for which rwo values i.e. 0 and 1 are supported.
 * try these function calls also in palce of already called function- 
 * myled.showmsg_single_static("font1",1);
 * For connection diagram visit https://goo.gl/I157Xv
 */

#include <TimerOne.h>
#include"SPI.h"
#include <ledP10.h>

int num1=0;
LedP10 myled;

void setup() 
{
  myled.init(3,4,8,9 ,1);
}

void loop() 
{
  myled.showmsg_single_static(num1,0);
  num1+=1;  
  delay(500);
}
