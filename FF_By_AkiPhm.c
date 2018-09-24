/*****************************************************
This program was produced by the
CodeWizardAVR V2.05.0 Professional
Project : Fish feeder with servo+Atmega8+relay control filter 
Date    : 15/10/2015
Author  : Aki Pham
Company : fb/AkiPham
Chip type               : ATmega8
Program type            : Application
AVR Core Clock frequency: 8.000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 256
*****************************************************/
#include <mega8.h>
#include <delay.h>
// I2C Bus functions
#asm
   .equ __i2c_port=0x15 ;PORTC           
   .equ __sda_bit=4
   .equ __scl_bit=5
#endasm
#include <i2c.h>         /// all this header already have in Codevison AVR
#include <ds1307.h>
#include <stdio.h>
#include <alcd.h>
/// declare all pin funtion control button + RESET
#define     MODE        PINC.0  // MODE switch each screen to display & follow button to change time
#define     SET         PINC.1  // SET for setting time for servo to start
#define     UP          PINC.2  // UP
#define     DOWN        PINC.3  // DOWN
#define     Feeder      PORTB.1  // servo conected in this port

eeprom unsigned char feedset;                   // set for turn alarm on                      
eeprom unsigned char feed_hour=0,feed_min=0;     // alarm setting hour-min
unsigned char buff[17];
unsigned char hour,min,sec,mode;


void time_display(unsigned char x,unsigned char y);   // time display
void feeder_lcd_display(unsigned char x,unsigned char y);    // feeder setting display
void fix_time(void); 
void feeding_ok();
void alarm_set();  // checking if alarm haven't set or not to display
unsigned char rtc_read(unsigned char address);
void rtc_write(unsigned char address, unsigned char data);

     /// sub program 


void time_display(unsigned char x,unsigned char y)   // time display
{
  lcd_gotoxy(x,y);
  sprintf(buff,"%02d",hour);
  lcd_puts(buff);
  lcd_putsf(":");
  sprintf(buff,"%02d",min);
  lcd_puts(buff);
  lcd_putsf(":");
  sprintf(buff,"%02d",sec);
  lcd_puts(buff);
}
//---------------------------------------------------------------
void feeder_lcd_display(unsigned char x,unsigned char y)    // feeder setting display
    {
  lcd_gotoxy(x,y);
  sprintf(buff,"%02d",feed_hour);
  lcd_puts(buff);
  lcd_putsf(":");
  sprintf(buff,"%02d",feed_min);
  lcd_puts(buff);
    }
//---------------------------------------------------------------

void alarm_set()  // checking if alarm haven't set or not to display
 {
    lcd_gotoxy(0,0) ;
    lcd_putsf("[");
    lcd_gotoxy(2,0) ;
    lcd_putsf("]");
          
        if(feedset==0) 
            {  
            lcd_gotoxy(1,0);
            lcd_putsf("*");
            }
        else
            {
            lcd_gotoxy(1,0);
            lcd_putsf(" ");
            } 
        
    }   
   
 
//////////////////////////////////////////////

//////////////////////////////////////////////
void fix_time(void)  // fix time
 {  
    if(sec > 60)
    {
        sec = 0;
        min++;
    }
    if(min > 60)
    {
        min= 0;
        hour++;        
    }
    if(hour > 24)
        hour = 0;
 } 
//---------------------------------------------------------------
void fix_time_alarm()
{
    if(feed_min > 60)
    {
        feed_min=0;
        feed_hour++;
    }
    if(feed_hour > 24)
     feed_hour=0;
}
//////////////////////////////////////////

void setup_time()
 {
   if(MODE==0)
    {
    if(mode==6)            //  che do
        { 
        mode=0;
        }
    else
        {
        lcd_clear();
        mode++;
        };
    while(!MODE);         // wait for release
    } 
       switch(mode)
    {
       case 1:             // MODE 1 : hour setup
        {     
            lcd_gotoxy(0,0);
            lcd_putsf("Hour setting >>>");
            time_display(5,1);
            if(!UP) hour++;  
            if(!DOWN) hour--;  
            delay_ms(10);
            fix_time();
            rtc_set_time(hour,min,sec);
            delay_ms(100); 
        } 
        break;
       case 2:             // MODE 2: minute setup
        {      
            lcd_gotoxy(0,0);
            lcd_putsf("Minute setting>>");
            time_display(5,1);
            if(!UP) 
             min++;    
            if(!DOWN)
             min--;
            delay_ms(10);     
            fix_time();   
            rtc_set_time(hour,min,sec);
            delay_ms(100);        
        }
        break;     
        case 3:  // feeder hour setup
        {   
            lcd_gotoxy(0,0);
            lcd_putsf("Feeder/Hour>>>");
            feeder_lcd_display(5,1);
            if(!UP)
             feed_hour++;   // hour alarm        
            if(!DOWN)
             feed_hour--;
            fix_time_alarm();
            delay_ms(100);                       
        }   
        break;
        case 4 :  // feeder minute setup                          
        {   
            lcd_gotoxy(0,0);
            lcd_putsf("Feeder/Minute>>>");
            feeder_lcd_display(5,1); 
            if(!UP)
             feed_min++;
            if(!DOWN)
             feed_min--;
            fix_time_alarm();
            delay_ms(100);             
        } 
        break;
        case 5: //screen display
        {    
            lcd_gotoxy(0,0);
            lcd_putsf("Setup complete...."); 
            lcd_gotoxy(0,1);
            lcd_putsf("Save starting>>>>>");
            delay_ms(150);                 // wait for release   
        } 
        break;           
       default: 
       lcd_clear();                 // wait for release
    }
 }
   

void feeding_ok()
{
if(feed_hour==hour&&feed_min==min&&feedset==0&&mode==0&&sec<6)  
    {    
        lcd_gotoxy(0,0);
        lcd_putsf("Feeding on process ");  
        lcd_gotoxy(0,1);
        lcd_putsf("FB/AkiPham "); 
        DDRB.1=1;
        OCR1A=1470;  
        delay_ms(1000);
        OCR1A=530;
        delay_ms(2000);
        OCR1A=2410;   
        delay_ms(3000);
        DDRB.1=0;
    } 
    else
        DDRB.1=0;       
 } 


 //////////
void check_feeder()
{
if(SET==0)
    {
    feedset=~feedset;
    while(!SET);
    };
}
///////

void main(void)
{

PORTB=0x00;
DDRB=0x00;

PORTC=0x0F;
DDRC=0x00;

PORTD=0x00;
DDRD=0x00;

//////// pwm
// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 1000.000 kHz
// Mode: Ph. correct PWM top=ICR1
TCCR1A=0xA2;
TCCR1B=0x1A;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x4E;
ICR1L=0x20;
ICR1=20000;
ACSR=0x80;
SFIOR=0x00;

// I2C Bus initialization
i2c_init();
rtc_init(0,0,0);
lcd_init(16);
#asm("sei")
lcd_clear();
hour=0;min=0;sec=0;mode=0;
rtc_write(0x00,40);
       
       lcd_gotoxy(0,1);
       lcd_putsf("Aki's Feeder PJ"); 
       lcd_gotoxy(5,0);
       lcd_putsf("FB/AkiPham"); 
       delay_ms(1000);     
while (1)
 { 
    sec= rtc_read(0x00);
    delay_ms(100);
    rtc_get_time(&hour,&min,&sec);                
    feeding_ok();
    setup_time();                       
    check_feeder();
    if(mode==0)                        
       { 
        alarm_set();
        time_display(7,0);        
        lcd_gotoxy(0,1);
        lcd_putsf("Press MODE>>SET UP"); 
        delay_ms(100);
        DDRB.1=0;
       }
       else                               
       {  
        setup_time();              
       };
      }           
 }
