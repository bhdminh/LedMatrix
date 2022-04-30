/*To display two counters of height 8 pixels.
 * function showmsg_double_static  takes three arguments, first two can be either pointer to string or integer indepedently. Third argument is to select font for which only 
 * one value is supported currently i.e. 0.
 * For connection diagram https://goo.gl/I157Xv
 */
 
#include <TimerOne.h>
#include"SPI.h"
#include <ledP10.h>
  
int num1=0,num2=1;
LedP10 myled;

void setup() 
{
  myled.init(3,4,8,9 ,1);
}

void loop() 
{  
  myled.showmsg_double_static(num1,num2,0);
  num1+=1;
  num2+=2;
  delay(500);
}
