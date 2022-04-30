
#include "Arduino.h"
#include<avr/pgmspace.h>
#include"SPI.h"

class LedP10
{
  public:
    LedP10();
	void showmsg_single_scroll(char msg[], int no_of_times,int speed, int font);
	void showmsg_double_scroll(char msg1_[],char msg2_[],int no_of_times1,int no_of_times2, int speed1_,int speed2, int font_);
	void showmsg_single_static(char msg_[],int font_);
	void showmsg_single_static(long int msg_,int font_);
	
	void showmsg_double_static(char msg1_[],char msg2_[],int font_);
	void showmsg_double_static(char msg1_[],long int msg2_,int font_);
	void showmsg_double_static(long int msg1_,char msg2_[],int font_);
	void showmsg_double_static(long int msg1_,long int msg2_,int font_);
	void init(int A, int B, int storepin, int oe, int no_of_panels);
	void setbrightness(uint8_t brightness_);
	
  private:	
	static void  callback();
	static void  callback2();
	static void  callback3();
	static void  callback4();
	static void showgroup(int p, int q);
	static void init_internal();
	static void init();
    static void slide();
	static void  slidesingle();
	static void filldisplaysingle();
	static void filldisplaysingle_static();
	static void filldisplaydouble();
	static void filldisplaydouble_static();
	static void showone();
	static void store();
	static char *msg,*msg1,*msg2;
	 static const uint8_t * font_lib;
	static uint8_t *dislay[2];
	
	char sts_dis[10];
	char sts_dis1[10];
	char sts_dis2[10];
	static int always;
	static int always1;
	static int always2;
    static int speed;
	static int speed1;
	static int speed2;
	static int speedcount;
	static int speedcount1;
	static int speedcount2;
	static int font;
	static int filled;
	static int index;
	static int filled1;
	static int filled2;
	static int index1;
	static int index2;
	static int numberofpanels;
	static int buffersize;
	static int storepin ;
	static int outputenablepin ;
	static int b ;
	static int a ;
	static int scroll;
	static int scroll1;
	static int scroll2;
	static int scrollcount;
	static int scrollcount1;
	static int scrollcount2;
	static int x;
	static int y;
	static uint8_t brightness;
	#define INF -1
};