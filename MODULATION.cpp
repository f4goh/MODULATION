/********************************************************************************************
 * MODULATION Arduino library
 * Created 11/6/2016
 * Anthony LE CREN f4goh@orange.fr 
 * thanks to Christophe f4goj for hfbeacon
 * BY NC SA
 * Instance :
 *
 * Functions :
 *
 *******************************************************************************************/
 

#include <MODULATION.h>


MODULATION Mod;

MODULATION::MODULATION(){
 rsidTxEnable = 1;
 wsprSymbGen = 0;
}

void MODULATION::begin(int freq, byte bf) {
  bf_pin=bf;
  pinMode(bf_pin,OUTPUT);
  analogWrite(bf_pin, 0);
  refclk=13157.9;      // measured 
  centerFreq=freq;
  
ddsReg[0]=MODULATION::computeDdsWord((double) centerFreq);
ddsReg[1]=MODULATION::computeDdsWord((double) centerFreq + 1.4548);
ddsReg[2]=MODULATION::computeDdsWord((double) centerFreq + 2*1.4548);
ddsReg[3]=MODULATION::computeDdsWord((double) centerFreq + 3*1.4548);
ddsReg[4]=MODULATION::computeDdsWord((double) centerFreq+170);
}


   

/********************************************************
 * Send a bit into an FM modulation
 ********************************************************/
 
void MODULATION::send_bit(int tempo)
{
countPtr=0;
int countPtrPrec=0;
while(countPtrPrec<tempo){
if (countPtrPrec<countPtr) {
	countPtrPrec=countPtr;
}
}
countPtr=0;
//digitalWrite(13,digitalRead(13)^1);
}


void MODULATION::sinus()
{

const static byte sinusTable[512] PROGMEM = {128,129,131,132,134,135,137,138,140,141,143,145,146,148,149,151,152,154,155,157,158,160,161,163,164,166,167,169,170,172,173,175,176,178,179,180,182,183,185,186,
                                             187,189,190,191,193,194,195,197,198,199,201,202,203,204,206,207,208,209,210,212,213,214,215,216,217,218,219,221,222,223,224,225,226,227,228,229,230,230,231,232,
                                             233,234,235,236,236,237,238,239,240,240,241,242,242,243,244,244,245,245,246,247,247,248,248,249,249,249,250,250,251,251,251,252,252,252,253,253,253,253,254,254,
                                             254,254,254,254,254,254,254,254,255,254,254,254,254,254,254,254,254,254,254,253,253,253,253,252,252,252,251,251,251,250,250,249,249,249,248,248,247,247,246,245,
                                             245,244,244,243,242,242,241,240,240,239,238,237,236,236,235,234,233,232,231,230,230,229,228,227,226,225,224,223,222,221,219,218,217,216,215,214,213,212,210,209,
                                             208,207,206,204,203,202,201,199,198,197,195,194,193,191,190,189,187,186,185,183,182,180,179,178,176,175,173,172,170,169,167,166,164,163,161,160,158,157,155,154,
                                             152,151,149,148,146,145,143,141,140,138,137,135,134,132,131,129,127,126,124,123,121,120,118,117,115,114,112,110,109,107,106,104,103,101,100,98,97,95,94,92,91,89,
                                             88,86,85,83,82,80,79,77,76,75,73,72,70,69,68,66,65,64,62,61,60,58,57,56,54,53,52,51,49,48,47,46,45,43,42,41,40,39,38,37,36,34,33,32,31,30,29,28,27,26,25,25,24,23,
                                             22,21,20,19,19,18,17,16,15,15,14,13,13,12,11,11,10,10,9,8,8,7,7,6,6,6,5,5,4,4,4,3,3,3,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,3,3,3,4,4,4,5,5,6,
                                             6,6,7,7,8,8,9,10,10,11,11,12,13,13,14,15,15,16,17,18,19,19,20,21,22,23,24,25,25,26,27,28,29,30,31,32,33,34,36,37,38,39,40,41,42,43,45,46,47,48,49,51,52,53,54,56,57,
                                             58,60,61,62,64,65,66,68,69,70,72,73,75,76,77,79,80,82,83,85,86,88,89,91,92,94,95,97,98,100,101,103,104,106,107,109,110,112,114,115,117,118,120,121,123,124,126};
											 
											 

 ddsAccu=ddsAccu+ddsWord; // soft DDS, phase accu with 32 bits
 sinusPtr=ddsAccu >> 23; 
 sinusPtr=((sinusPtr+phase)&0x1ff);		//add phase

analogWrite(bf_pin,pgm_read_byte(&(sinusTable[sinusPtr])));
countPtr++;
}

unsigned long MODULATION::computeDdsWord(double freq)
{
return pow(2,32)*freq/refclk;
}



/********************************************************
 * RSID
 ********************************************************/
void MODULATION::rsidToggle(boolean rsidEnable){
 if(rsidEnable == true)
 {
  rsidTxEnable = 1;
 }
 else
 {
  rsidTxEnable = 0;
 }
}

void MODULATION::rsidTx(int modeRsid){
 const static int RSID[8][16] PROGMEM = {
  {0,0,8,10,9,10,1,8,2,11,9,2,3,11,1,-83},     //bpsk31
  {0,2,3,12,13,14,12,1,13,2,15,15,3,0,14,-83}, //qpsk31
  {0,0,9,13,11,13,2,9,4,15,11,4,6,15,2,-83},   //bpsk63
  {0,0,1,7,2,7,3,1,6,4,2,6,5,4,3,-83},         //qpsk63
  {0,9,7,15,0,6,14,14,1,8,9,8,6,1,7,0},        //RTTY45
  {0,2,1,2,9,0,10,3,1,10,11,3,9,8,8,-66},      //feld hell
  {0,0,11,3,15,3,4,11,8,7,15,8,12,7,4,-83},    //bpsk125
  {0,0,3,9,6,9,5,3,10,12,6,10,15,12,5,-83}     //qpsk125
 };

  int a = 0;
  int offset;
  
  offset = int(pgm_read_word(&RSID[modeRsid][15])); // used to center RSID and transmission
  
  //delay(500);   // 0.5s for RSID start
  for  (int element=0; element<15; element++) {   // For Each Element in the message
   a = int(pgm_read_word(&RSID[modeRsid][element])); // get the numerical ASCII Code
    	ddsWord=MODULATION::computeDdsWord((double) centerFreq+ offset + (a * 10.766));  //112us
		MODULATION::send_bit(1220);			//13157.9*0.092768=1220  // baud rate 92,88ms
  }
}


/***************************************************************************
 * CW
 ***************************************************************************/
void MODULATION::cwTx(char * stringCw, int cwWpm){
 const static int morseVaricode[2][59] PROGMEM = {
  {0,212,72,0,144,0,128,120,176,180,0,80,204,132,84,144,248,120,56,24,8,0,128,192,224,240,224,168,0,136,0,48,104,64,128,160,128,0,32,192,0,0,112,160,64,192,128,224,96,208,64,0,128,32,16,96,144,176,192},
  {7,6,5,0,4,0,4,6,5,6,0,5,6,6,6,5,5,5,5,5,5,5,5,5,5,5,6,6,0,5,0,6,6,2,4,4,3,1,4,3,4,2,4,3,4,2,2,3,4,4,3,3,1,3,4,3,4,4,4}
 }; 

 unsigned long tempo = 1200 / cwWpm; // Duration of 1 dot
 byte nb_bits,val;
 int d;
 int c = *stringCw++;
  while(c != '\0'){
  c = toupper(c); // Uppercase
  if(c == 32){       // Space character between words in string
   ddsWord=0;
   MODULATION::send_bit((13157 * tempo * 7)/1000);
  }
  else if (c > 32 && c < 91) {
   c = c - 32;
   d = int(pgm_read_word(&morseVaricode[0][c]));    // Get CW varicode    
   nb_bits = int(pgm_read_word(&morseVaricode[1][c])); // Get CW varicode length
   if(nb_bits != 0){ // Number of bits = 0 -> invalid character #%<>
    for(int b = 7; b > 7 - nb_bits; b--){ // Send CW character, each bit represents a symbol (0 for dot, 1 for dash) MSB first 
     val=bitRead(d,b);  //look varicode
      ddsWord=ddsReg[0];
      MODULATION::send_bit((13157 * (tempo + 2 * tempo * val))/1000); // A dot length or a dash length (3 times the dot)
	  ddsWord=0;
      MODULATION::send_bit((13157 * tempo)/1000);  // between symbols in a character      
     }
    }
    ddsWord=0; // 3 dots length spacing
    MODULATION::send_bit((13157 * tempo * 3)/1000);  // between characters in a word
   }
  c = *stringCw++;  // Next caracter in string
 }
}


/********************************************************
 * PSK
 ********************************************************/

void MODULATION::pskTx(char * stringPsk, int modePsk, int baudsPsk)
{
 const static int PskVaricode[2][128] PROGMEM = {
  {683,731,749,887,747,863,751,765,767,239,29,879,733,31,885,939,759,757,941,943,859,875,877,
   855,891,893,951,853,861,955,763,895,1,511,351,501,475,725,699,383,251,247,367,479,117,53,
   87,431,183,189,237,255,375,347,363,429,427,439,245,445,493,85,471,687,701,125,235,173,181,
   119,219,253,341,127,509,381,215,187,221,171,213,477,175,111,109,343,437,349,373,379,685,503,
   495,507,703,365,735,11,95,47,45,3,61,91,43,13,491,191,27,59,15,7,63,447,21,23,5,55,123,107,
   223,93,469,695,443,693,727,949},
  {10,10,10,10,10,10,10,10,10,8,5,10,10,5,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
   10,1,9,9,9,9,10,10,9,8,8,9,9,7,6,7,9,8,8,8,8,9,9,9,9,9,9,8,9,9,7,9,10,10,7,8,8,8,7,8,8,9,7,
   9,9,8,8,8,8,8,9,8,7,7,9,9,9,9,9,10,9,9,9,10,9,10,4,7,6,6,2,6,7,6,4,9,8,5,6,4,3,6,9,5,5,3,6,
   7,7,8,7,9,10,9,10,10,10}
  };

 const static int QpskConvol[32] PROGMEM = {256,128,-128,0,-128,0,256,128,0,-128,128,256,128,256,0,-128,128,256,0,-128,0,-128,128,256,-128,0,256,128,256,128,-128,0};
 
 
 int shreg = 0;  // Shift register qpsk	
 phase = 0;
 if(rsidTxEnable == 1)
 {
                                                         // 0 bpsk31
                                                         // 1 qpsk31
                                                         // 2 bpsk63
   rsidTx((baudsPsk >> 4) - (modePsk == 'B')); // 3 qpsk63
                                                         // 6 bpsk125
                                                         // 7 qpsk125
 }     
 if (baudsPsk==31) baudsPsk=424;
 if (baudsPsk==63) baudsPsk=208;
 if (baudsPsk==125) baudsPsk=105; //13157.9*(1/125)=105

 pskIdle(baudsPsk);  // A little idle on start of transmission for AFC capture
   
 byte nb_bits,val;
 int d,e;
 int c = *stringPsk++;
 ddsWord=ddsReg[0];
 while (c != '\0')
 {
  d = int(pgm_read_word(&PskVaricode[0][c]));    // Get PSK varicode    
  nb_bits = int(pgm_read_word(&PskVaricode[1][c])); // Get PSK varicode length
  d <<= 2; //add 00 on lsb for spacing between caracters
  e = d;
  for(int b = nb_bits + 2; b >= 0; b--) //send car in psk
  {
   val=bitRead(e,b); //look varicode
   if(modePsk == 'B')  // BPSK mode
   {
    if (val == 0)
	{
	 if (phase==0) phase=256; else phase=0;	//swapp phase 0/180°
    }
   }
   else if(modePsk == 'Q'){       // QPSK mode
    shreg = (shreg << 1) | val;  // Loading shift register with next bit
    d=(int)int(pgm_read_word(&QpskConvol[shreg & 31])); // Get the phase shift from convolution code of 5 bits in shit register
    phase = (phase + d) & 0x1ff;  // Phase shifting
   }
   MODULATION::send_bit(baudsPsk); 		
   digitalWrite(13,digitalRead(13)^1);
  }
  c = *stringPsk++;  // Next caracter in string
 }
 pskIdle(baudsPsk); // A little idle to end the transmission
}

void MODULATION::pskIdle(unsigned int bauds)
{
  ddsWord=ddsReg[0];
 for(int n = 0; n < 50; n++)
 {
   if (phase==0) phase=256; else phase=0;
  MODULATION::send_bit(bauds); 
 // digitalWrite(13,digitalRead(13)^1);
 }
}

/********************************************
 * RTTY
 ********************************************/ 
void MODULATION::rttyTx(char * stringRtty)
{
 const static int TableRtty[59] PROGMEM = {4,22,17,5,18,0,11,26,30,9,0,0,6,24,7,23,13,29,25,16,10,1,21,28,12,3,14,15,0,0,0,19,0,24,19,14,18,16,22,11,5,12,26,30,9,7,6,3,13,29,10,20,1,28,15,25,23,21,17};

 int signlett = 1;  // RTTY Baudot signs/letters tables toggle
 char c;
 c = *stringRtty++;

 if(rsidTxEnable == 1)
 {
 rsidTx(4);
 }
 
 while ( c != '\0')
 {
 //Serial.print(c);
  c = toupper(int(c)); // Uppercase
  if(c == 10) // Line Feed
  {
   rttyTxByte(8);
  }
  else if(c == 13) // Carriage Return
  {
   rttyTxByte(2);
  }
  else if(c == 32) // Space
  { 
   rttyTxByte(4);
  }
  else if (c > 32 && c < 91)
  {
   c = c - 32;
   if(c < 33)
   {
    if (signlett == 1)
    {
	 signlett = 0;      // toggle form signs to letters table
     rttyTxByte(27);  //
    }
   }
   else if(signlett == 0)
   {
    signlett = 1;          // toggle form letters to signs table
    rttyTxByte(31);      //
   }
  rttyTxByte(int(pgm_read_word(&TableRtty[int(c)])));  // Send the 5 bits word
  }
  c = *stringRtty++;  // Next character in string
 }
}



void MODULATION::rttyTxByte(char c){
 int val;
 c = (c << 2)+3; 
 for(int b = 7; b >= 0; b--) // MSB first
 {
  val = bitRead(c,b); // Read 1 bit
 if (val==0) ddsWord=ddsReg[0]; else ddsWord=ddsReg[4]; // Let's transmit (bit 1 is shifted)
 MODULATION::send_bit(292); //baud rate
 }
}


/***********************************************************************************
* Hellschreiber 122.5 bauds => 8,888ms
* http://brainwagon.org/2012/01/11/hellduino-sending-hellschreiber-from-an-arduino
************************************************************************************/

void MODULATION::hellTx( char * stringHell)
{
const static word GlyphTab[59][8] PROGMEM = {
  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},{0x1f9c, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},{0x0330, 0x0ffc, 0x0330, 0x0ffc, 0x0330, 0x0000, 0x0000},
  {0x078c, 0x0ccc, 0x1ffe, 0x0ccc, 0x0c78, 0x0000, 0x0000},{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
  {0x01e0, 0x0738, 0x1c0e, 0x0000, 0x0000, 0x0000, 0x0000},{0x1c0e, 0x0738, 0x01e0, 0x0000, 0x0000, 0x0000, 0x0000},{0x018c, 0x0198, 0x0ff0, 0x0198, 0x018c, 0x0000, 0x0000},{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},{0x001c, 0x001c, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},{0x001c, 0x0070, 0x01c0, 0x0700, 0x1c00, 0x0000, 0x0000},
  {0x07f8, 0x0c0c, 0x0c0c, 0x0c0c, 0x07f8, 0x0000, 0x0000},{0x0300, 0x0600, 0x0ffc, 0x0000, 0x0000, 0x0000, 0x0000},{0x061c, 0x0c3c, 0x0ccc, 0x078c, 0x000c, 0x0000, 0x0000},{0x0006, 0x1806, 0x198c, 0x1f98, 0x00f0, 0x0000, 0x0000},
  {0x1fe0, 0x0060, 0x0060, 0x0ffc, 0x0060, 0x0000, 0x0000},{0x000c, 0x000c, 0x1f8c, 0x1998, 0x18f0, 0x0000, 0x0000},{0x07fc, 0x0c66, 0x18c6, 0x00c6, 0x007c, 0x0000, 0x0000},{0x181c, 0x1870, 0x19c0, 0x1f00, 0x1c00, 0x0000, 0x0000},
  {0x0f3c, 0x19e6, 0x18c6, 0x19e6, 0x0f3c, 0x0000, 0x0000},{0x0f80, 0x18c6, 0x18cc, 0x18cc, 0x0ff0, 0x0000, 0x0000},{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},{0x1800, 0x1800, 0x19ce, 0x1f00, 0x0000, 0x0000, 0x0000},
  {0x01f0, 0x0208, 0x04e4, 0x0514, 0x0514, 0x03e0, 0x0000},{0x07fc, 0x0e60, 0x0c60, 0x0e60, 0x07fc, 0x0000, 0x0000},{0x0c0c, 0x0ffc, 0x0ccc, 0x0ccc, 0x0738, 0x0000, 0x0000},{0x0ffc, 0x0c0c, 0x0c0c, 0x0c0c, 0x0c0c, 0x0000, 0x0000},
  {0x0c0c, 0x0ffc, 0x0c0c, 0x0c0c, 0x07f8, 0x0000, 0x0000},{0x0ffc, 0x0ccc, 0x0ccc, 0x0c0c, 0x0c0c, 0x0000, 0x0000},{0x0ffc, 0x0cc0, 0x0cc0, 0x0c00, 0x0c00, 0x0000, 0x0000},{0x0ffc, 0x0c0c, 0x0c0c, 0x0ccc, 0x0cfc, 0x0000, 0x0000},
  {0x0ffc, 0x00c0, 0x00c0, 0x00c0, 0x0ffc, 0x0000, 0x0000},{0x0c0c, 0x0c0c, 0x0ffc, 0x0c0c, 0x0c0c, 0x0000, 0x0000},{0x003c, 0x000c, 0x000c, 0x000c, 0x0ffc, 0x0000, 0x0000},{0x0ffc, 0x00c0, 0x00e0, 0x0330, 0x0e1c, 0x0000, 0x0000},
  {0x0ffc, 0x000c, 0x000c, 0x000c, 0x000c, 0x0000, 0x0000},{0x0ffc, 0x0600, 0x0300, 0x0600, 0x0ffc, 0x0000, 0x0000},{0x0ffc, 0x0700, 0x01c0, 0x0070, 0x0ffc, 0x0000, 0x0000},{0x0ffc, 0x0c0c, 0x0c0c, 0x0c0c, 0x0ffc, 0x0000, 0x0000},
  {0x0c0c, 0x0ffc, 0x0ccc, 0x0cc0, 0x0780, 0x0000, 0x0000},{0x0ffc, 0x0c0c, 0x0c3c, 0x0ffc, 0x000f, 0x0000, 0x0000},{0x0ffc, 0x0cc0, 0x0cc0, 0x0cf0, 0x079c, 0x0000, 0x0000},{0x078c, 0x0ccc, 0x0ccc, 0x0ccc, 0x0c78, 0x0000, 0x0000},
  {0x0c00, 0x0c00, 0x0ffc, 0x0c00, 0x0c00, 0x0000, 0x0000},{0x0ff8, 0x000c, 0x000c, 0x000c, 0x0ff8, 0x0000, 0x0000},{0x0ffc, 0x0038, 0x00e0, 0x0380, 0x0e00, 0x0000, 0x0000},{0x0ff8, 0x000c, 0x00f8, 0x000c, 0x0ff8, 0x0000, 0x0000},
  {0x0e1c, 0x0330, 0x01e0, 0x0330, 0x0e1c, 0x0000, 0x0000},{0x0e00, 0x0380, 0x00fc, 0x0380, 0x0e00, 0x0000, 0x0000},{0x0c1c, 0x0c7c, 0x0ccc, 0x0f8c, 0x0e0c, 0x0000, 0x0000}
 };
 int val;
 char ch;
 int n;
 word fbits ;
 if(rsidTxEnable == 1)
 {
 rsidTx(5);
 }
 ch = *stringHell++;
 while (ch != '\0')
 {
  ch = toupper(int(ch)); // Uppercase
  if(ch >= 32 && ch <= 90) // Character is in the range of ASCII space to Z
  {
   ch -= 32;  // Character number starting at 0
   for (int i = 0; i < 7; i++) // Scanning each 7 columns of glyph
   {  
	fbits = int(pgm_read_word(&GlyphTab[int(ch)][i]));  // Get each column of glyph
    for (int b = 0; b < 14; b++) // Scanning each 14 rows
    {
	 val = bitRead(fbits,b);  // Get binary state of pixel
	 if (val==1) {					// Gives the baud rate. 4045µs minus DDS loading time, 4002µs in SPI mode, 3438µs in software serial mode
			ddsWord=ddsReg[0];
			MODULATION::send_bit(53);
	 }
	  else { //pwm à 0
			ddsWord=0;
			MODULATION::send_bit(53);
	  }
	  for (int n=0;n<81;n++){__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");}
    }
   }
  }
  ch = *stringHell++; // Next character in string
 }
}

/*******************************************************
* WSPR
********************************************************/

void MODULATION::wsprTx(char * callWsprTx, char * locWsprTx, char * powWsprTx)
{
 if(wsprSymbGen == 0)
 {
  MODULATION::wsprEncode(callWsprTx, locWsprTx, powWsprTx);
 }
 if(wsprSymbGen == 1)
 {
  int a = 0;
  for (int element = 0; element < 162; element++) {    // For each element in the message
    a = int(wsprSymb[element]); //   get the numerical Code 0,1,2,3
   		ddsWord=ddsReg[a];
		MODULATION::send_bit(8973);			//13157.9*0.682=8973.68
	//digitalWrite(13,digitalRead(13)^1);    
  }
 
 }
}

/*************************************************************************
* WSPR message encoder
* http://www.g4jnt.com/Coding%5CWSPR_Coding_Process.pdf
**************************************************************************/
void MODULATION::wsprEncode(char * callWsprProc, char * locWsprProc, char * powWsprProc)
{
char callsign[] = "      ";
char encode_call[] = "       ";
char locator[] = "    ";
char power[] = "00";
byte pwr_lvl;
char ch;
int counter = 0;

ch = *callWsprProc++;
while(ch != '\0')
{
 callsign[counter] = ch;
 ch = *callWsprProc++;
 counter++;
}
counter = 0;
ch = *locWsprProc++;
while(ch != '\0')
{
 locator[counter] = ch;
 ch = *locWsprProc++;
 counter++;
}

counter = 0;
ch = *powWsprProc++;
while(ch != '\0')
{
 power[counter] = ch;
 ch = *powWsprProc++;
 counter++;
}

 for(int i = 0; i < 6; i++){
  if(callsign[i] == ' ')
  {
   callsign[i] = callsign[i] + 4;
  }
  else if(callsign[i] > 47 && callsign[i] < 58)
  {
   callsign[i] = callsign[i] - 48;
  }
  else if(callsign[i] > 64 && callsign[i] < 91)
  {
   callsign[i] = callsign[i] - 55;
  }
  else if(callsign[i] > 96 && callsign[i] < 123)
  {
   callsign[i] = callsign[i] - 87;
  }
 }

 if(callsign[2] > 9){
  encode_call[0] = 36;
  for(int i = 0; i < 5; i++)
  {
   encode_call[i + 1] = callsign [i];
  }
 }

/*****************************************************************
* Callsign encoding
*****************************************************************/
unsigned long N;

N = encode_call[0];
N = 36 * N + encode_call[1];
N = 10 * N + encode_call[2];
N = 27 * N + encode_call[3] - 10;
N = 27 * N + encode_call[4] - 10;
N = 27 * N + encode_call[5] - 10;

/*****************************************************************
* Locator encoding
*****************************************************************/
unsigned long M1;

for(int i = 0; i < 4; i++){
 if(locator[i] > 64 && locator[i] < 83){
  locator[i] = locator[i] - 65;
 }
 else if(locator[i] > 96 && locator[i] < 123)
  {
   locator[i] = locator[i] - 97;
  }
 else if(locator[i] > 47 && locator[i] < 58){
  locator[i] = locator[i] - 48;
 }
}

M1 = 180 * (179 - 10 * locator[0] - locator[2]) + 10 * locator[1] + locator[3];

/*****************************************************************
* Power encoding
*****************************************************************/
pwr_lvl = 10 * (power[0] - '0') + power[1] - '0';

/*****************************************************************
* Locator + power packing
*****************************************************************/
unsigned long M;
M = 128 * M1 + pwr_lvl + 64;

/*****************************************************************
* Bit packing
*****************************************************************/
unsigned char c[11];

c[0] = N >> 20; // 8 MSB
c[1] = N >> 12; // 8 next
c[2] = N >> 4;  // 8 next
c[3] = N << 4;  // 4 LSB -> 4 MSB c[3]
c[3] = c[3] + (0x0f & M >> 18);
c[4] = M >> 10;
c[5] = M >> 2;
c[6] = M << 6;
c[7] = 0;
c[8] = 0;
c[9] = 0;
c[10] = 0;

/*****************************************************************
* Convolutional encoding
*****************************************************************/
unsigned long reg = 0;
int conv;
int conv_byte = 0;
int compt_bits = 0;
byte S[162];

conv = c[0];
int m = 0;

for(int i = 0; i < 81; i++){
 if(compt_bits % 8 == 0){
  conv = c[conv_byte];
  conv_byte++;
 } 
 reg = reg | bitRead(conv, 7);
 
 S[m++] = parity(reg & 0xf2d05351);
 S[m++] = parity(reg & 0xe4613c47);

 compt_bits++;
 conv = conv << 1;
 reg = reg << 1;
}


/*****************************************************************
* Interleaving
*****************************************************************/
 
byte D[162];
int p = 0;
byte temp = 0;
int j = 0;

 for(int i = 0; i < 256; i++)
 {
  j = i;
  temp = j;
  for(int k = 0; k < 8; k++){
   bitWrite(j, k , bitRead(temp, 7 - k));
  }
  if(j < 162){
   D[j] = S[p];
   p++;
  }
 }

/*****************************************************************
* Merge with sync vector
*****************************************************************/
 const static byte Sync[] PROGMEM = {1,1,0,0,0,0,0,0,1,0,0,0,1,1,1,0,0,0,1,0,0,1,0,1,1,1,1,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,1,
               0,0,1,1,0,1,0,0,0,1,1,0,1,0,0,0,0,1,1,0,1,0,1,0,1,0,1,0,0,1,0,0,1,0,1,1,0,0,0,1,1,0,1,0,1,0,0,0,1,0,
               0,0,0,0,1,0,0,1,0,0,1,1,1,0,1,1,0,0,1,1,0,1,0,0,0,1,1,1,0,0,0,0,0,1,0,1,0,0,1,1,0,0,0,0,0,0,0,1,1,0,
               1,0,1,1,0,0,0,1,1,0,0,0};

 for(int i = 0; i < 162; i++){
  wsprSymb[i] = pgm_read_word(&Sync[i]) + 2 * D[i];
 }
 wsprSymbGen = 1;
}

/*****************************************************************
* Parity calculator
*****************************************************************/
byte MODULATION::parity(unsigned long tempo){
 byte par = 0;
  for(int k = 0; k < 32; k++){
   par = par ^ (tempo & 0x01);
   tempo = tempo >> 1;
  }
 return par;
}
