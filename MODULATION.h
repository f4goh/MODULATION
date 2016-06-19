/********************************************************************************************
 * MODULATION Arduino library
 * Created 11/6/2016
 * Anthony LE CREN f4goh@orange.fr 
 * thanks to Christophe Caiveau f4goj@free.fr 
 * BY NC SA
 *
 * Instance :
 *
 * Functions :
 *
 *******************************************************************************************/
 
 
 
#ifndef MODULATION_H
#define MODULATION_H
#include <Arduino.h>

class MODULATION
{
public:
  MODULATION();

  byte bf_pin;

  void begin(int freq, byte bf);

  void rsidToggle(boolean rsidEnable);
  void cwTx(char * stringCw, int cwWpm);
  void rttyTx(char * stringRtty);
  void hellTx(char * stringHell);
  void wsprTx(char * callWsprTx, char * locWsprTx, char * powWsprTx);
  void wsprEncode(char * callWsprProc, char * locWsprProc, char * powWsprProc);
  void pskTx(char * stringPsk, int modePsk, int baudsPsk);    

  byte wsprSymb[162];
  int wsprSymbGen;
  void rttyTxByte (char c);

  void sinus();


private:
  void send_bit(int tempo);
  void rsidTx(int modeRsid);
  void pskIdle(unsigned int bauds);
  byte parity(unsigned long tempo);
  byte rsidTxEnable;
  volatile int phase;
  unsigned long ddsReg[5];
  double refclk;
  volatile unsigned long ddsAccu;   // phase accumulator
  volatile unsigned long ddsWord;
  volatile unsigned int sinusPtr;
  volatile int countPtr;
  volatile int shift;
  unsigned long computeDdsWord(double freq);
  unsigned int centerFreq;
};


extern MODULATION Mod;

#endif
