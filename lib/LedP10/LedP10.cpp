#include"SPI.h"
#include "Arduino.h"
#include"Arial_black_16.h"
#include"double_font2.h"
#include"led_half_1.h"
#include<avr/pgmspace.h>
#include"LedP10.h"
#include <TimerOne.h>
#include<string.h>

	 char * LedP10 :: msg=NULL;
	 char * LedP10 :: msg1=NULL;
	 char * LedP10 :: msg2=NULL;
	 const uint8_t * LedP10 ::   font_lib;
	 uint8_t * LedP10 :: dislay[2]={NULL};
     int  LedP10 :: speed=5;
	 int LedP10 ::  font;
	 int LedP10 ::  filled=0;
	 int LedP10 ::  index=0;
	 int LedP10 ::  filled1=0;
	 int LedP10 ::  filled2=0;
	 int LedP10 ::  index1=0;
	 int LedP10 ::  index2=0;
	 int LedP10 ::  numberofpanels=1;
	 int LedP10 ::  buffersize=49;
	 int LedP10 ::  storepin =8;
	 int LedP10 ::  outputenablepin=9 ;
	 int LedP10 ::  b =4;
	 int LedP10 ::  a =3;
	 int LedP10 ::  scroll=-1;
	 int LedP10 ::  scroll1=-1;
	 int LedP10 ::  scroll2=-1;
	 int LedP10 ::  scrollcount=0;
	 int LedP10 ::  scrollcount1=0;
	 int LedP10 ::  scrollcount2=0;
	 int LedP10 ::  x=0;
	 int LedP10 ::  y=0;
	 int LedP10 ::  speed1=5;
	 int LedP10 ::  speed2=5;
	 int LedP10 ::  speedcount=0;
	 int LedP10 ::  speedcount1=0;
	 int LedP10 ::  speedcount2=0;
	 int LedP10 ::  always=1;
	 int LedP10 ::  always1=0;
	 int LedP10 ::  always2=1;
	 uint8_t LedP10 ::  brightness=255;
	 LedP10 :: LedP10()
{}
void LedP10 :: init_internal()
{
	pinMode(outputenablepin,OUTPUT);
	pinMode(a,OUTPUT);
	pinMode(b,OUTPUT);
	pinMode(10,OUTPUT);

	pinMode(storepin,OUTPUT);
	digitalWrite(a, LOW);
	digitalWrite(b, LOW);
	SPI.beginTransaction(SPISettings(1152000, MSBFIRST, SPI_MODE0));
	SPI.begin();
	digitalWrite(outputenablepin,LOW);
	scroll=-1;
	scroll1=-1;
	scroll2=-1;
}

void LedP10 :: store()
{
  digitalWrite(storepin, HIGH);
  digitalWrite(storepin, LOW);
}

void LedP10 :: init(int A, int B, int store_pin, int oe, int no_of_panels)
{
	a=A;
	b=B;
	storepin=store_pin;
	outputenablepin=oe;
	numberofpanels=no_of_panels;
	buffersize=17+numberofpanels*32;
	dislay[0]=(uint8_t *)malloc(buffersize*sizeof(uint8_t));
	dislay[1]=(uint8_t *)malloc(buffersize*sizeof(uint8_t));
	for(int i=0;i<buffersize;i++)
	{
	dislay[0][i]=0x00;
	dislay[1][i]=0x00;
	}
	LedP10 ::init_internal();
} 
void LedP10 :: filldisplaysingle_static()
{
	filled=0;
	index=0;
	char nextchar;
	int no_of_char=pgm_read_byte_near(font_lib+5);
	int first_char = pgm_read_byte_near(font_lib+4);
	uint16_t widthpos,width,datapos; 
	while(index!=strlen(msg)&&filled<32*numberofpanels)
	{
		nextchar=msg[index++];
		widthpos= 6+nextchar-first_char;
		uint16_t sum=0;
		for(int i=0;i<nextchar-first_char;i++)
		{
			sum+= 2*pgm_read_byte_near(font_lib+6+i);
		}
		datapos= 6+no_of_char+sum;
		width=pgm_read_byte_near(font_lib+widthpos);
		if(filled+width+1<=32*numberofpanels)
		{
			word count=0;
			for(count=0;count<width;count++)
			{
				dislay[0][filled]=pgm_read_byte_near(font_lib+datapos+count);
				dislay[1][filled]=pgm_read_byte_near(font_lib+datapos+count+width);
				(filled)++;
			}
			dislay[0][filled]=0x00;
			dislay[1][filled]=0x00;
			(filled)++;  
		}
		
	}
	while(filled<32*numberofpanels)
	{
		dislay[0][filled]=0x00;
			dislay[1][filled]=0x00;
			(filled)++;  
	}
}

void LedP10 :: filldisplaysingle()
{
	char nextchar;
	uint16_t widthpos,width,datapos;
	int no_of_char=pgm_read_byte_near(font_lib+5);
	int first_char = pgm_read_byte_near(font_lib+4);
	while(filled<buffersize&&(scroll>0||always==1))
	{
		if(index==strlen(msg))
		{
			scroll--;
			index=0;
			nextchar=' ';
		}
		else
		{
			nextchar=msg[index++];
		}
		widthpos= 6+nextchar-first_char;
		uint16_t sum=0;
		for(int i=0;i<nextchar-first_char;i++)
		{
			sum+= 2*pgm_read_byte_near(font_lib+6+i);
		}
		datapos= 6+no_of_char+sum;
		width=pgm_read_byte_near(font_lib+widthpos);
		if(filled+width+1<=buffersize)
		{
			word count=0;
			for(count=0;count<width;count++)
			{
				dislay[0][filled]=pgm_read_byte_near(font_lib+datapos+count);
				dislay[1][filled]=pgm_read_byte_near(font_lib+datapos+count+width);
				(filled)++;
			}
			dislay[0][filled]=0x00;
			dislay[1][filled]=0x00;
			(filled)++;  
		}
		else
		{
			if(index>0)
			index--;
			else
			{
				index=strlen(msg);
				scroll++;
			}
			break;
		}
	}
}

void LedP10 :: filldisplaydouble_static()
{
	filled1=0;filled2=0;index1=0;index2=0;
	char nextchar1,nextchar2;
	int no_of_char=pgm_read_byte_near(font_lib+5);
	int first_char = pgm_read_byte_near(font_lib+4);
	uint16_t widthpos1,width1,datapos1,widthpos2,width2,datapos2;
	while(index1!=strlen(msg1)&&filled1<32*numberofpanels)
	{
		nextchar1=msg1[index1++];
		widthpos1= 6+nextchar1-first_char;
		uint16_t sum1=0;
		for(int i=0;i<nextchar1-first_char;i++)
		{
			sum1+= pgm_read_byte_near(font_lib+6+i);
		}
		datapos1= 6+no_of_char+sum1;
		width1=pgm_read_byte_near(font_lib+widthpos1);
		if(filled1+width1+1<=32*numberofpanels)
		{
			word count1=0;
			for(count1=0;count1<width1;count1++)
			{
				dislay[0][filled1]=pgm_read_byte_near(font_lib+datapos1+count1);
				(filled1)++;
			}
			dislay[0][filled1]=0x00;
		}
		
	}
	while(filled1<32*numberofpanels)
	{
		dislay[0][filled1++]=0x00; 
	}
	while(index2!=strlen(msg2)&&filled2<32*numberofpanels)
	{
		nextchar2=msg2[index2++];
		widthpos2= 6+nextchar2-first_char;
		uint16_t sum2=0;
		for(int i=0;i<nextchar2-first_char;i++)
		{
			sum2+= pgm_read_byte_near(font_lib+6+i);
		}
		datapos2= 6+no_of_char+sum2;
		width2=pgm_read_byte_near(font_lib+widthpos2);
		if(filled2+width2+1<=32*numberofpanels)
		{
			word count2=0;
			for(count2=0;count2<width2;count2++)
			{
				dislay[1][filled2]=pgm_read_byte_near(font_lib+datapos2+count2);
				(filled2)++;
			}
			dislay[1][filled2++]=0x00;
		}
		
	}
	while(filled2<32*numberofpanels)
	{
		dislay[1][filled2++]=0x00; 
	}
	
}


void LedP10 :: filldisplaydouble()
{
	char nextchar1,nextchar2;
	uint16_t widthpos1,width1,datapos1,widthpos2,width2,datapos2;
	int no_of_char=pgm_read_byte_near(font_lib+5);
	int first_char = pgm_read_byte_near(font_lib+4); 
	while(filled1<buffersize&&(scroll1>0||always1==1))
	{
		if(index1==strlen(msg1))
		{
			scroll1--;
			index1=0;
			nextchar1=' ';
		}
		else
		{
			nextchar1=msg1[index1++];
		}
		widthpos1= 6+nextchar1-first_char;
		uint16_t sum1=0;
		for(int i=0;i<nextchar1-first_char;i++)
		{
			sum1+= pgm_read_byte_near(font_lib+6+i);
		}
		datapos1= 6+no_of_char+sum1;
		width1=pgm_read_byte_near(font_lib+widthpos1);
		if(filled1+width1+1<=buffersize)
		{
			word count1=0;
			for(count1=0;count1<width1;count1++)
			{
				dislay[0][filled1]=pgm_read_byte_near(font_lib+datapos1+count1);
				(filled1)++;
			}
			dislay[0][filled1]=0x00;
			(filled1)++;  
		}
		else
		{
			if(index1>0)
			index1--;
			else
			{
			scroll1++;
			index1=strlen(msg1);
			}
			break;
		}
	}
	while(filled2<buffersize&&(scroll2>0||always2==1))
	{
		if(index2==strlen(msg2))
		{
			scroll2--;
			index2=0;
			nextchar2=' ';
		}
		else
		{
			nextchar2=msg2[index2++];
		}
		widthpos2= 6+nextchar2-first_char;
		uint16_t sum2=0;
		for(int i=0;i<nextchar2-first_char;i++)
		{
			sum2+= pgm_read_byte_near(font_lib+6+i);
		}
		datapos2= 6+no_of_char+sum2;
		width2=pgm_read_byte_near(font_lib+widthpos2);
		if(filled2+width2+1<=buffersize)
		{
			word count2=0;
			for(count2=0;count2<width2;count2++)
			{
				dislay[1][filled2]=pgm_read_byte_near(font_lib+datapos2+count2);
				(filled2)++;
			}
			dislay[1][filled2]=0x00;
			(filled2)++;  
		}
		else
		{
			if(index2>0)
			index2--;
			else
			{
			index2=strlen(msg2);
			scroll2++;
			}
			break;
		}
	}
}

void LedP10 :: slide()
{
    int i;
	if(speed1!=0)
	{
		speedcount1++;
		if(speedcount1==30/speed1)
		{
			speedcount1=0;
			for(i=0;i<buffersize-1 ;i++)
			{
				dislay[0][i]=  dislay[0][i+1];
			}
			filled1--;
		}
	} 
	if(speed2!=0)
	{
		speedcount2++;
		if(speedcount2==30/speed2)
		{
			speedcount2=0;
			for(i=0;i<buffersize-1 ;i++)
			{
				dislay[1][i]=  dislay[1][i+1];
			}
			filled2--;
		}
	}
	
	//if(speed1==speed2&&speed1!=0)
	filled--;
}

void LedP10 ::slidesingle()
{
	int i;
	if(speed!=0)
	{
		speedcount++;
		if(speedcount==30/speed)
		{
			speedcount=0;
			for(i=0;i<buffersize-1 ;i++)
			{
				dislay[0][i]=  dislay[0][i+1];
				dislay[1][i]=  dislay[1][i+1];
			}
			filled--;
		}
	} 
	else
	{
		speedcount++;
		if(speedcount==30/speed)
		{
			speedcount=0;
			for(i=0;i<buffersize-1 ;i++)
			{
				dislay[0][i]=  dislay[0][i];
				dislay[1][i]=  dislay[1][i];
			}
			filled--;
		}	
	}
}
void LedP10 :: showgroup(int p,int q)
{
	int i,j,k,row,col,byt;
	byte temp,data=0x00;
    for(col=0;col<4*numberofpanels;col++)
    {
        row=12+p+2*q;
        byt=row/8;
        for(i=0;i<4;i++)
        {
			data=0x00;
			for(k=0;k<8;k++)
			{
				data=data<<1;
				temp=0x01;
				temp&=(dislay[row/8][8*col+k]>>(row%8));
				data|=temp;
			}
			row-=4;
			byt=row/8;
			SPI.transfer(~data);
        }
    }   
    digitalWrite(outputenablepin,LOW);
    LedP10 ::store(); 
}

void LedP10 :: showmsg_single_scroll(char msg_[],int no_of_times, int speed_, int font_)
{
	if(font_==0)
		font_lib=Arial_Black_16;
	else if(font_==1)
		font_lib=Arial_14;
	scroll=	no_of_times;
	msg=msg_;
	speed=speed_;
	speed1=speed;
	speed2=speed;
	speedcount1=0;
	speedcount2=0;
	font=font_;
	if(no_of_times<0)
	always=1;
	else
	always=0;
	Timer1.initialize(100);
	Timer1.attachInterrupt(callback4);
}

 
 
 void LedP10 :: callback()
{
	digitalWrite(outputenablepin,LOW);
	LedP10 ::filldisplaysingle_static();
	LedP10 ::showone();
	
}
 void LedP10 :: callback2()
{
	digitalWrite(outputenablepin,LOW);
	LedP10 ::filldisplaydouble_static();
	LedP10 ::showone();
	
}
void LedP10 ::  showone()
{
	if(x==0&&y==0)
	{
		LedP10 ::showgroup(0,0);
		digitalWrite(a,LOW);
		digitalWrite(b,LOW);
		analogWrite(outputenablepin, brightness);
		x=1;y=0;
	}
	else if(x==1&&y==0)
	{
	LedP10 ::showgroup(1,0);
    digitalWrite(a,HIGH);
    digitalWrite(b,LOW);
    analogWrite(outputenablepin,brightness);
	x=0;y=1;
	}
	else if(x==0&&y==1)
	{
		LedP10 ::showgroup(0,1);
		digitalWrite(a,LOW);
		digitalWrite(b,HIGH);
		analogWrite(outputenablepin,brightness);
		x=1;y=1;
	}
	else if(x==1&&y==1)
	{
		LedP10 ::showgroup(1,1);
		digitalWrite(a,HIGH);
		digitalWrite(b,HIGH);
		analogWrite(outputenablepin, brightness);
		x=0;y=0;
	}
}
void LedP10 :: showmsg_single_static(long int msg_,int font_)
{
	String thisString1 = String(msg_);
	thisString1.toCharArray(sts_dis,thisString1.length()+1);
	//free(&thisString1);
	//showmsg_single_static(newmsg1,font_);
	digitalWrite(outputenablepin,LOW);
	Timer1.stop();
	if(font_==0)
		font_lib=Arial_Black_16;
	else if(font_==1)
		font_lib=Arial_14;
	msg=sts_dis;
	font=font_;
	filled=0;
	Timer1.initialize(5500);
	Timer1.attachInterrupt(callback);
	LedP10 ::filldisplaysingle_static();
}


	
void LedP10 :: showmsg_single_static(char msg_[],int font_)
{
	digitalWrite(outputenablepin,LOW);
	Timer1.stop();
	if(font_==0)
		font_lib=Arial_Black_16;
	else if(font_==1)
		font_lib=Arial_14;
	msg=msg_;
	font=font_;
	filled=0;
	Timer1.initialize(5500);
	Timer1.attachInterrupt(callback);
	LedP10 ::filldisplaysingle_static();
}
void LedP10 :: showmsg_double_static(long int msg1_,long int msg2_,int font_)
{
	String thisString1 = String(msg1_);
	thisString1.toCharArray(sts_dis1,thisString1.length()+1);
	String thisString2 = String(msg2_);  
  thisString2.toCharArray(sts_dis2,thisString2.length()+1);
  showmsg_double_static(sts_dis1,sts_dis2, font_);
}

void LedP10 :: showmsg_double_static(long int msg1_,char msg2_[],int font_)
{
	String thisString1 = String(msg1_);
	thisString1.toCharArray(sts_dis1,thisString1.length()+1);
	
  showmsg_double_static(sts_dis1,msg2_, font_);
}

void LedP10 :: showmsg_double_static(char msg1_[],long int msg2_,int font_)
{
	String thisString2 = String(msg2_);  
  thisString2.toCharArray(sts_dis2,thisString2.length()+1);
  showmsg_double_static(msg1_,sts_dis2, font_);
}
void LedP10 :: showmsg_double_static(char msg1_[],char msg2_[],int font_)
{
	digitalWrite(outputenablepin,LOW);
	Timer1.stop();
	if(font_==0)
		font_lib=font_lib=Led_half_1;
	else if(font_==1)
		{}
	msg1=msg1_;
	msg2=msg2_;
	font=font_;
	filled1=0;
	filled2=0;
	Timer1.initialize(6500);
	Timer1.attachInterrupt(callback2);
	LedP10 ::filldisplaydouble_static();
}
void LedP10 :: callback3()
{
	
			LedP10 ::showone();
			if(x==1&&y==1)
			LedP10 ::slide();
			LedP10 ::filldisplaydouble();
			
	if(filled1==0&&filled2==0&&scroll1==0&&scroll2==0)
	{
		digitalWrite(outputenablepin,LOW);
		Timer1.stop();
	}
}
void LedP10 :: callback4()
{
	
			LedP10 ::showone();
			if(x==1&&y==1)
			{
				LedP10 ::slidesingle();
			}
						
			filldisplaysingle();
			
	if(filled==0&&scroll==0)
	{
		digitalWrite(outputenablepin,LOW);
		Timer1.stop();
	}
}
void LedP10 :: showmsg_double_scroll(char msg1_[],char msg2_[],int no_of_times1,int no_of_times2, int speed1_,int speed2_, int font_)
{
	if(font_==0)
		font_lib=font_lib=Led_half_1;
	else if(font_==1)
		{}
	msg1=msg1_;
	msg2=msg2_;
	speed1=speed1_;
	speed2=speed2_;
	font=font_;
	scroll1=	no_of_times1;
	scroll2=	no_of_times2;
	if(scroll1<0)
	always1=1;
	else
	always1=0;
	if(scroll2<0)
	always2=1;
	else
	always2=0;
	LedP10 ::filldisplaydouble();
	Timer1.initialize(100);
	Timer1.attachInterrupt(callback3);
}

void LedP10 :: setbrightness(uint8_t brightness_)
{
	brightness=brightness_;
}
