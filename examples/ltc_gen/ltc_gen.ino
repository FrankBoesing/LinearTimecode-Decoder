/*
  Linear Timecode generation and decoding combined

  Please modify as needed

  (c) Frank B, 2020
  (pls keep copyright-info)

  To enable decoding:
  #define USE_LTC_INPUT
  and connect pin1 to pinA2 (=pin 16)

  Licence: MIT
*/

#define USE_LTC_INPUT //comment-out for generation only
float fps = 24;
const int ltcPin = 1;
const int syncPin = 0;

#include <TimeLib.h>


#ifdef USE_LTC_INPUT
/*
   Connect ltc
*/
#include <Audio.h>
#include <analyze_ltc.h>
AudioInputAnalog         adc1;
AudioAnalyzeLTC          ltc1;
AudioConnection          patchCord2(adc1, 0, ltc1, 0);

#else
struct ltcframe_t {
  uint64_t data;
  uint16_t sync;
};
#endif

IntervalTimer ltcTimer;
IntervalTimer fpsTimer;

volatile int clkCnt = 0;
ltcframe_t ltc;
float ltcTimer_freq;

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void genLtc() {
  static int state = 0;

  if (clkCnt++ >= 2 * 80) {
    //ltcTimer.end();
    clkCnt = 0;
    return;
  }

  int bitval;
  int bitcount = clkCnt / 2;

  ltcframe_t ltct;
  ltct = ltc;

  if (bitcount < 64) {
    bitval = (int) (ltct.data >> bitcount) & 0x01;
  } else {
    bitval = (int) (ltct.sync >> (bitcount - 64)) & 0x01; //backward
  }

  if ( (bitval == 1) || ( (bitval == 0) && (clkCnt & 0x01) ) == 0) {
    state = !state; // toggle state
    digitalWriteFast(ltcPin, state);
  }

}

//https://www.geeksforgeeks.org/program-to-find-parity/
inline
int getParity(uint64_t n)
{
  int parity = 0;
  while (n)
  {
    parity = !parity;
    n     = n & (n - 1);
  }
  return parity & 0x01;
}


/*

   Gets called by rising edge of syncPin
*/
void startLtc() {


  ltcTimer.begin(genLtc, ltcTimer_freq);
  ltcTimer.priority(0);

  int t, t10;
  uint64_t data = ltc.data;
  clkCnt = 0;

  //get frame number:
  t = ((data >> 8) & 0x03) * 10 + (data & 0x0f);

  //zero out ltc data, leave  d+c flags and user-bits untouched:
  data &= 0xf0f0f0f0f0f0fcf0ULL;
  //data = 0;
  //inc frame-number:
  //TODO: realize "drop frame numbering" (when D-Flag is set)
  t++; 
  if (t >= (int) fps) t = 0;
  //set frame number:
  t10 = t / 10;
  data |= (t10 & 0x03) << 8;
  data |= ((t - t10 * 10)) << 0;

  //set time:
  t = second();
  t10 = t / 10;
  data |= (t10 & 0x07) << 24; //Seconds tens
  data |= ((t - t10 * 10) & 0x0f) << 16;

  t = minute();
  t10 = t / 10;
  data |= (uint64_t)(t10 & 0x07) << 40; //minute tens
  data |= (uint64_t)((t - t10 * 10) & 0x0f) << 32;

  t = hour();
  t10 = t / 10;
  data |= (uint64_t)(t10 & 0x03) << 56; //hour tens
  data |= (uint64_t)((t - t10 * 10) & 0x0f) << 48;

  //set parity:
  int parity = (!getParity(data)) & 0x01;
  if ((int) fps == 25) {
    data |= (uint64_t) parity << 59;
  } else {
    data |= (uint64_t) parity << 27;
  }

  ltc.data = data;

}

void initLtcData()
{
  ltc.sync = 0xBFFC;
  ltc.data = (uint64_t) 0; //<- place your initial data here
}

void genFpsSync() {
  static int state = 0;
  state = !state; // toggle state
  digitalWriteFast(syncPin, state);
}

void loop() {

  while (1) {
#ifdef USE_LTC_INPUT
    if (ltc1.available()) {
      ltcframe_t ltcframe = ltc1.read();
      Serial.printf("%02d:%02d:%02d.%02d\n", ltc1.hour(&ltcframe), ltc1.minute(&ltcframe), ltc1.second(&ltcframe), ltc1.frame(&ltcframe));
    }
#else
    Serial.printf(" %02d:%02d:%02d\n", hour(), minute(), second());
    delay(1000);
#endif
  }
}

void setup() {
#ifdef USE_LTC_INPUT
  AudioMemory(4);
#endif
  setSyncProvider(getTeensy3Time);
  pinMode(syncPin, OUTPUT);
  pinMode(ltcPin, OUTPUT);
  Serial.begin(9600);
  ltcTimer_freq = (1.0f / (2 * 80 * (fps ))) * 1000000.0f - 0.125f;// -0.125: make it a tiny bit faster than needed to allow syncing
  
  initLtcData();
  //set frame number to last frame to force roll-over with new second()
  ltc.data &= 0xf0f0f0f0f0f0fcf0ULL; //delete all dynamic data in frame
  int t, t10;
  t = fps;
  t10 = t / 10;
  ltc.data |= (t10 & 0x03) << 8;
  ltc.data |= ((t - t10 * 10));    
  //now wait for seconds-change 
  int secs = second();
  while (secs == second()) {;}  
  
  //start timer and pin-interrupt:
  fpsTimer.begin(&genFpsSync, (1.0f / (2 * fps)) * 1000000.0f);
  fpsTimer.priority(0);  
  attachInterrupt(digitalPinToInterrupt(syncPin), &startLtc, RISING);
  NVIC_SET_PRIORITY(88, 0); //set GPIO-INT-Priority for Pin 1
}
