/* Audio Library for Teensy 3.X
   Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com

   This Code:
   Copyright (c) 2019, Frank Bösing, f.boesing (at) gmx.de

   Please support PJRC's efforts to develop open source software by purchasing
   Teensy or other PJRC products.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice, development funding notice, and this permission
   notice shall be included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

// https://forum.pjrc.com/threads/41584-Audio-Library-for-Linear-Timecode-(LTC)

#include <Arduino.h>
#include "analyze_ltc.h"

inline void AudioAnalyzeLTC::decodeBitstream(unsigned newbit) {
  static ltcframe_t ltc;
  static int bitcounter = 0;
  static bool forward = true;
  static uint32_t lastts = 0;
  
  if (bitcounter < 64) {

    if (forward)
      ltc.data = (ltc.data >> 1) | ((uint64_t) newbit << 63);
    else
      ltc.data = (ltc.data << 1) | newbit;

    bitcounter++;

  } else {

    ltc.sync = (ltc.sync << 1) | newbit;

    if (ltc.sync == 0x3FFD) {
      ltc.timestampfirstedge = lastts;
      lastts = micros();
      ltcframe = ltc;
      new_output = true;
      bitcounter = 0;
      forward = true;
    } else if (ltc.sync == 0xBFFC) {
      ltc.timestampfirstedge = lastts;
      lastts = micros();
      ltcframe = ltc;
      new_output = true;
      bitcounter = 0;
      forward = false;
    }

  }
}

void AudioAnalyzeLTC::update(void)
{
  audio_block_t *block;
  const int16_t *p, *end;

  block = receiveReadOnly();
  if (!block) {
    return;
  }
  p = block->data;
  end = p + AUDIO_BLOCK_SAMPLES;

  static int clkcnt = 0, avclk = 0, newbit = 0;
  static bool clkstate = false, clkstatelast = false;
  int minsample = 32767, maxsample = -32768;

  do {
    int16_t sample = *p++;

    if (sample < minsample) minsample = sample;
    else if (sample > maxsample) maxsample = sample;

    clkstatelast = clkstate;
    clkstate = (sample > avsample);

    if (clkstate == clkstatelast)
      clkcnt++;
    else {

      if ( clkcnt < avclk ) {
        avclk = clkcnt * 3 / 2;
        if (++newbit == 2) {
          newbit = 0;
          decodeBitstream(1);
        }
      } else {

        avclk = (clkcnt + clkcnt / 2) / 2;
        newbit = 0;
        decodeBitstream(0);
      }
      clkcnt = 0;
    }
  } while (p < end);
  release(block);
  avsample = (maxsample + minsample) / 2;
}
