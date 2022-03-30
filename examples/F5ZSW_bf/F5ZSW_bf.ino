/*sample code for F5ZSW BF version
  Anthony LE CREN F4GOH@orange.fr
  Created 23/3/2022
  the program send cw and ft8 sequence 4 times per minutes
  with GPS to have perfect timing
*/


#include <JTEncode.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <TimerOne.h>


//NANO pinout
#define txPin 6 //tx pin into RX GPS connection
#define rxPin 7 //rx pin into TX GPS connection

#define LED 8   //output pin led
#define PTT   3    // PTT pin. This is active low.
#define bfPin 5     //afsk output must be PD5 


//#define debugGPS //enable print all nmea sentences on serial monitor
#define debugDDS //enable debug DDS with some key commands as 0,1,2,3 on serial monitor (see debug function)
//#define debugSYMBOLS

#define CW_FREQUENCY 10133000  //same fequency for test
#define FT8_FREQUENCY 10133000  //

#define CW_TXT " VVV F5ZSW JN23MK"

#define CALL "F5ZSW"        //callsign
#define DBM 20              //power in dbm
#define GPS_BAUD_RATE 9600
#define PC_BAUD_RATE  115200
#define PWM_GAIN 140      //continous voltage ctrl for mosfet 110=1.2V  -> 1K and 4.7K (210)

#define MESSAGE "CQ F5ZSW JN23"
#define FT8_TONE_SPACING        6.25f
#define FT8_DELAY               160

SoftwareSerial ss(rxPin, txPin); // RX, TX for GPS
JTEncode jtencode;
TinyGPS gps;


enum mode {
  CW,
  FT8_CQ,
};

byte tableSeconds[] = {0, 15, 30, 45}; //seconds to transmit
mode tableMode[] = {CW, CW, CW, FT8_CQ};

long factor = -1500;		//adjust frequency to wspr band
uint8_t symbols[WSPR_SYMBOL_COUNT];    //buffer memeory for WSPR or ft8 encoding



volatile int phase;
unsigned long ddsReg[8];
double refclk;
volatile unsigned long ddsAccu;   // phase accumulator
volatile unsigned long ddsWord;
volatile unsigned int sinusPtr;
volatile int countPtr;
volatile int shift;
unsigned int centerFreq;
byte save_TIMSK0;
byte save_PCICR;


void setup() {
  Serial.begin(PC_BAUD_RATE);
  Serial.print("Hello ");
  Serial.println(CALL );
  pinMode(LED, OUTPUT);
  pinMode(bfPin, OUTPUT);
  analogWrite(bfPin, 0);
  pinMode(PTT, OUTPUT);
  digitalWrite(PTT, LOW);
  ss.begin(GPS_BAUD_RATE);
  delay(2000);


  //latlng2loc(48.605776, -4.550759, 0);
  //Serial.print("LOCATOR : ");
  //Serial.println(pos.locator);
  //Serial.println(pos.pwrDbm);

  Timer1.initialize(76);    //µs  fe=13200 hz so TE=76µs 13157.9 mesured

  begin(1500);

}


//main loop
void loop() {

#ifdef debugDDS
  char c;
  if (Serial.available() > 0) {
    c = Serial.read();
    Serial.println(c);
    debug(c);
  }
#endif
  lectGps();
}


/********************************************************
  WSPR loop with time sync
 ********************************************************/

void  lectGps()
{
  bool newData = false;

  // for (unsigned long start = millis(); millis() - start < 1000;)
  // {
  while (ss.available())
  {
    char c = ss.read();
#ifdef debugGPS
    Serial.write(c);
#endif
    if (gps.encode(c)) // Did a new valid sentence come in?
      newData = true;
  }
  // }
  if (newData)
  {
    unsigned long age;
    int year;
    byte month, day, hour, minute, second, hundredths;
    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
    if (age == TinyGPS::GPS_INVALID_AGE)
      Serial.println("*******    *******    ");
    else
    {
      char sz[32];
      char index;
      sprintf(sz, "%02d:%02d:%02d   ", hour, minute, second);
      Serial.println(sz);
      digitalWrite(LED, digitalRead(LED) ^ 1);
      index = syncSeconds(second);
      if ((index >= 0)  && (second % 15 == 0)) {
        mode mtx = tableMode[index];
        switch (mtx) {
          case CW :
            txCW(CW_FREQUENCY, CW_TXT, 30);
            break;
          case FT8_CQ :
            txFt8();
            break;
        }
      }
    }
  }
}

char syncSeconds(int m) {
  int i;
  char ret = -1;
  for (i = 0; i < sizeof(tableSeconds); i++) {
    if (m == tableSeconds[i]) {
      ret = i;
    }
  }
  return ret;
}

/********************************************************
  Debug function
********************************************************/


void debug(char c)
{
  switch (c)
  {
    case 'c' :
      txCW(CW_FREQUENCY, CW_TXT, 30);
      break;
    case 'f' :
      txFt8();    //ft8 cq
      break;

  }
}
/**************************
   cw
 **************************/
void txCW(long freqCw, char * stringCw, int cwWpm) {
  const static int morseVaricode[2][59] PROGMEM = {
    {0, 212, 72, 0, 144, 0, 128, 120, 176, 180, 0, 80, 204, 132, 84, 144, 248, 120, 56, 24, 8, 0, 128, 192, 224, 240, 224, 168, 0, 136, 0, 48, 104, 64, 128, 160, 128, 0, 32, 192, 0, 0, 112, 160, 64, 192, 128, 224, 96, 208, 64, 0, 128, 32, 16, 96, 144, 176, 192},
    {7, 6, 5, 0, 4, 0, 4, 6, 5, 6, 0, 5, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 0, 5, 0, 6, 6, 2, 4, 4, 3, 1, 4, 3, 4, 2, 4, 3, 4, 2, 2, 3, 4, 4, 3, 3, 1, 3, 4, 3, 4, 4, 4}
  };

  unsigned long tempo = 1200 / cwWpm; // Duration of 1 dot
  byte nb_bits, val;
  int d;
  int c = *stringCw++;
  txingOn();
  while (c != '\0') {
    c = toupper(c); // Uppercase
    if (c == 32) {     // Space character between words in string
      ddsWord = 0;
      send_bit((13157 * tempo * 7) / 1000);
    }
    else if (c > 32 && c < 91) {
      c = c - 32;
      d = int(pgm_read_word(&morseVaricode[0][c]));    // Get CW varicode
      nb_bits = int(pgm_read_word(&morseVaricode[1][c])); // Get CW varicode length
      if (nb_bits != 0) { // Number of bits = 0 -> invalid character #%<>
        for (int b = 7; b > 7 - nb_bits; b--) { // Send CW character, each bit represents a symbol (0 for dot, 1 for dash) MSB first
          val = bitRead(d, b); //look varicode
          ddsWord = ddsReg[0];
          send_bit((13157 * (tempo + 2 * tempo * val)) / 1000); // A dot length or a dash length (3 times the dot)
          ddsWord = 0;
          send_bit((13157 * tempo) / 1000); // between symbols in a character
        }
      }
      ddsWord = 0; // 3 dots length spacing
      send_bit((13157 * tempo * 3) / 1000); // between characters in a word
    }
    c = *stringCw++;  // Next caracter in string
  }
  txingOff();
}




/**************************
   ft8
 **************************/

void txFt8() {

  Serial.println(MESSAGE);
  memset(symbols, 0, FT8_SYMBOL_COUNT);   //clear memory
  jtencode.ft8_encode(MESSAGE, symbols);
#ifdef debugSYMBOLS
  int n, lf;
  lf = 0;
  for (n = 0; n < FT8_SYMBOL_COUNT; n++) {   //print symbols on serial monitor
    if (lf % 16 == 0) {
      Serial.println();
      Serial.print(n);
      Serial.print(": ");
    }
    lf++;
    Serial.print(symbols[n]);
    Serial.print(',');
  }
  Serial.println();
#endif
  sendFt8(FT8_FREQUENCY);
}

void sendFt8(long freq) {
  int a = 0;
  txingOn();

  for (int element = 0; element < FT8_SYMBOL_COUNT; element++) {    // For each element in the message
    a = int(symbols[element]); //   get the numerical ASCII Code
    //setfreq((double) freq + (double) a * FT8_TONE_SPACING, 0);
    //delay(FT8_DELAY);
    ddsWord = ddsReg[a];
    send_bit(2105);     //13157.9*0.160=2105.264

#ifdef debugSYMBOLS
    Serial.print(a);
#endif
    digitalWrite(LED, digitalRead(LED) ^ 1);
  }
  txingOff();
}

void txingOn()
{

  digitalWrite(PTT, HIGH);            //ptt on
  delay(50);                         //delay before sending data
  TCCR0B = TCCR0B & 0b11111000 | 1;    //switch to 62500 HZ PWM frequency
  save_TIMSK0 = TIMSK0;                //save Timer 0 register
  TIMSK0 = 0;                          //disable Timer 0
  save_PCICR = PCICR;                  //save external pin interrupt register
  PCICR = 0;                           //disable external pin interrupt
  Timer1.attachInterrupt(sinus);   //warp interrupt in library
}

void txingOff() {
  digitalWrite(PTT, LOW);              //PTT off
  Timer1.detachInterrupt();            //disable timer1 interrupt
  analogWrite(bfPin, 0);                //PWM at 0
  TCCR0B = TCCR0B & 0b11111000 | 3;    //register return to normal
  TIMSK0 = save_TIMSK0;
  PCICR = save_PCICR;
  Serial.println("EOT");
}




void begin(int freq) {
  refclk = 13157.9;    // measured
  centerFreq = freq;

  ddsReg[0] = computeDdsWord((double) centerFreq);
  ddsReg[1] = computeDdsWord((double) centerFreq + FT8_TONE_SPACING);
  ddsReg[2] = computeDdsWord((double) centerFreq + 2 * FT8_TONE_SPACING);
  ddsReg[3] = computeDdsWord((double) centerFreq + 3 * FT8_TONE_SPACING);
  ddsReg[4] = computeDdsWord((double) centerFreq + 4 * FT8_TONE_SPACING);
  ddsReg[5] = computeDdsWord((double) centerFreq + 5 * FT8_TONE_SPACING);
  ddsReg[6] = computeDdsWord((double) centerFreq + 6 * FT8_TONE_SPACING);
  ddsReg[7] = computeDdsWord((double) centerFreq + 7 * FT8_TONE_SPACING);
  
}




/********************************************************
   Send a bit into an FM modulation
 ********************************************************/

void send_bit(int tempo)
{
  countPtr = 0;
  int countPtrPrec = 0;
  while (countPtrPrec < tempo) {
    if (countPtrPrec < countPtr) {
      countPtrPrec = countPtr;
    }
  }
  countPtr = 0;
  //digitalWrite(13,digitalRead(13)^1);
}


void sinus()
{

  const static byte sinusTable[512] PROGMEM = {128, 129, 131, 132, 134, 135, 137, 138, 140, 141, 143, 145, 146, 148, 149, 151, 152, 154, 155, 157, 158, 160, 161, 163, 164, 166, 167, 169, 170, 172, 173, 175, 176, 178, 179, 180, 182, 183, 185, 186,
                                               187, 189, 190, 191, 193, 194, 195, 197, 198, 199, 201, 202, 203, 204, 206, 207, 208, 209, 210, 212, 213, 214, 215, 216, 217, 218, 219, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 230, 231, 232,
                                               233, 234, 235, 236, 236, 237, 238, 239, 240, 240, 241, 242, 242, 243, 244, 244, 245, 245, 246, 247, 247, 248, 248, 249, 249, 249, 250, 250, 251, 251, 251, 252, 252, 252, 253, 253, 253, 253, 254, 254,
                                               254, 254, 254, 254, 254, 254, 254, 254, 255, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 253, 253, 253, 253, 252, 252, 252, 251, 251, 251, 250, 250, 249, 249, 249, 248, 248, 247, 247, 246, 245,
                                               245, 244, 244, 243, 242, 242, 241, 240, 240, 239, 238, 237, 236, 236, 235, 234, 233, 232, 231, 230, 230, 229, 228, 227, 226, 225, 224, 223, 222, 221, 219, 218, 217, 216, 215, 214, 213, 212, 210, 209,
                                               208, 207, 206, 204, 203, 202, 201, 199, 198, 197, 195, 194, 193, 191, 190, 189, 187, 186, 185, 183, 182, 180, 179, 178, 176, 175, 173, 172, 170, 169, 167, 166, 164, 163, 161, 160, 158, 157, 155, 154,
                                               152, 151, 149, 148, 146, 145, 143, 141, 140, 138, 137, 135, 134, 132, 131, 129, 127, 126, 124, 123, 121, 120, 118, 117, 115, 114, 112, 110, 109, 107, 106, 104, 103, 101, 100, 98, 97, 95, 94, 92, 91, 89,
                                               88, 86, 85, 83, 82, 80, 79, 77, 76, 75, 73, 72, 70, 69, 68, 66, 65, 64, 62, 61, 60, 58, 57, 56, 54, 53, 52, 51, 49, 48, 47, 46, 45, 43, 42, 41, 40, 39, 38, 37, 36, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 25, 24, 23,
                                               22, 21, 20, 19, 19, 18, 17, 16, 15, 15, 14, 13, 13, 12, 11, 11, 10, 10, 9, 8, 8, 7, 7, 6, 6, 6, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6,
                                               6, 6, 7, 7, 8, 8, 9, 10, 10, 11, 11, 12, 13, 13, 14, 15, 15, 16, 17, 18, 19, 19, 20, 21, 22, 23, 24, 25, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 36, 37, 38, 39, 40, 41, 42, 43, 45, 46, 47, 48, 49, 51, 52, 53, 54, 56, 57,
                                               58, 60, 61, 62, 64, 65, 66, 68, 69, 70, 72, 73, 75, 76, 77, 79, 80, 82, 83, 85, 86, 88, 89, 91, 92, 94, 95, 97, 98, 100, 101, 103, 104, 106, 107, 109, 110, 112, 114, 115, 117, 118, 120, 121, 123, 124, 126
                                              };



  ddsAccu = ddsAccu + ddsWord; // soft DDS, phase accu with 32 bits
  sinusPtr = ddsAccu >> 23;
  sinusPtr = ((sinusPtr + phase) & 0x1ff); //add phase

  analogWrite(bfPin, pgm_read_byte(&(sinusTable[sinusPtr])));
  countPtr++;
}

unsigned long computeDdsWord(double freq)
{
  return pow(2, 32) * freq / refclk;
}
