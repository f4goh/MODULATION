/*
 * F4GOH Anthony JN07DV
 * Use freely
 * hellschreiber from http://brainwagon.org/2012/01/11/hellduino-sending-hellschreiber-from-an-arduino/
 * Set the date and time by entering the following on the Arduino       
 * 
 * serial monitor:                                                      
 *    year,month,day,hour,minute,second,                                
 *                                                                      
 * Where                                                                
 *    year can be two or four digits,                                   
 *    month is 1-12,                                                    
 *    day is 1-31,                                                      
 *    hour is 0-23, and                                                 
 *    minute and second are 0-59.          
 *    2016,6,18,16,32,00,
 *    
 *    freq wspr
 *    80 m : 3.592600 
 *    40 m : 7.038600 
 *    30 m : 10.138700 
 *    20 m : 14.095600 
 *    15 m : 21.094600 
 */



#include <MODULATION.h>
#include <SoftwareSerial.h>
#include <TimerOne.h>
#include <DS3232RTC.h> //http://github.com/JChristensen/DS3232RTC
#include <Wire.h>
#include <Time.h>


tmElements_t tm; 


#define PTT   3    // PTT pin. This is active low.
#define bfPin 5     //afsk output must be PD5 


char txString[] = "...F4GOH F4GOH Beacon JN07DV JN07DV please report F4GOH@orange.fr F4GOH@orange.fr tks sk...";
char txCall[] = "F4GOH";
char txLoc[] = "JN07";
char txPow[] = "00";


#define TRX_RXD 6   // We *receive* Serial from the TRX on this pin.

#define TRX_TXD 7   // We *send* serial to the TRX on this pin.

SoftwareSerial civ(TRX_RXD, TRX_TXD);    //for CAT system

int pskSpeed=31;
char pskMode='B';

char trxPttOn[]="TX;";
char trxPttOff[]="RX;";
//char trx30m[]="FA00010144000;";

void setup() {
  Serial.begin(57600);
  pinMode(PTT, OUTPUT);
  digitalWrite(PTT, LOW);

  civ.begin(38400);

  Timer1.initialize(76);    //µs  fe=13200 hz so TE=76µs 13157.9 mesured

  Mod.begin(1500, bfPin);

  Mod.rsidToggle(0);

  //majRtc();    //uncomment to set RTC
}

void loop() {
   RTC.read(tm);

  if ((tm.Minute & 0x01) == 0 && tm.Second == 0) {
    txing(3);   //1 : rtty, 2: hellschreiber, 3: wspr, 4: cw, 5: psk and qpsk 31 63 125
  }
  Serial.print(tm.Hour);
  Serial.print(":");
  Serial.print(tm.Minute);
  Serial.print(":");
  Serial.println(tm.Second);
 }

void test()
{
  char car;
if (Serial.available()>0){
             car=Serial.read();
             if (car=='1') civ.print(trxPttOn);
             if (car=='0') civ.print(trxPttOff);
  Serial.print(car);
}
               
}

void txing(byte mod)
{
  byte save_TIMSK0;
  byte save_PCICR;

  digitalWrite(PTT, HIGH);            //ptt on
  civ.print(trxPttOn);
  delay(500);                         //delay before sending data
  TCCR0B = TCCR0B & 0b11111000 | 1;    //switch to 62500 HZ PWM frequency
  save_TIMSK0=TIMSK0;                  //save Timer 0 register
  TIMSK0 =0;                           //disable Timer 0
  save_PCICR=PCICR;                    //save external pin interrupt register
  PCICR = 0;                           //disable external pin interrupt
  Timer1.attachInterrupt(sinus_irq);   //warp interrupt in library
  switch (mod)
  {
  case  1: 
    Mod.rttyTx(txString);
    break;
  case  2: 
    Mod.hellTx(txString);
    break;
  case  3 :
    Mod.wsprTx(txCall, txLoc, txPow);
    break;
  case  4: 
    Mod.cwTx(txString, 18);
    break;
  case  5:  
    Mod.pskTx(txString, pskMode, pskSpeed);
    break;
  }   

  digitalWrite(PTT, LOW);              //PTT off  
  Timer1.detachInterrupt();            //disable timer1 interrupt
  analogWrite(bfPin, 0);                //PWM at 0
  TCCR0B = TCCR0B & 0b11111000 | 3;    //register return to normal
  TIMSK0 =save_TIMSK0;
  PCICR = save_PCICR;
  civ.begin(38400);
  civ.print(trxPttOff);
}

void sinus_irq()    //warp timer1 irq into DRAPRS lib
{
  Mod.sinus();
}


void majRtc()
{
  time_t t;
  //check for input to set the RTC, minimum length is 12, i.e. yy,m,d,h,m,s
  Serial.println(F("update time : format yy,m,d,h,m,s"));
  Serial.println(F("exemple : 2016,6,18,16,32,30, "));

  while (Serial.available () <= 12) {
  }
  //note that the tmElements_t Year member is an offset from 1970,
  //but the RTC wants the last two digits of the calendar year.
  //use the convenience macros from Time.h to do the conversions.
  int y = Serial.parseInt();
  if (y >= 100 && y < 1000)
    Serial.println(F("Error: Year must be two digits or four digits!"));
  else {
    if (y >= 1000)
      tm.Year = CalendarYrToTm(y);
    else    //(y < 100)
    tm.Year = y2kYearToTm(y);
    tm.Month = Serial.parseInt();
    tm.Day = Serial.parseInt();
    tm.Hour = Serial.parseInt();
    tm.Minute = Serial.parseInt();
    tm.Second = Serial.parseInt();
    t = makeTime(tm);
    RTC.set(t);        //use the time_t value to ensure correct weekday is set
    setTime(t);        
    while (Serial.available () > 0) Serial.read();
  }
}

