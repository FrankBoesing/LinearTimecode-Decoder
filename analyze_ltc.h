/* Audio Library for Teensy 3.X
   Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com

   This Code:
   Copyright (c) 2019, Frank Bösing, f.boesing (at) gmx.de

   Development of this audio library was funded by PJRC.COM, LLC by sales of
   Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
   open source software by purchasing Teensy or other PJRC products.

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


#ifndef analyze_ltc_h_
#define analyze_ltc_h_

#include "Arduino.h"
#include "AudioStream.h"

struct ltcframe_t {
  uint64_t data;
  uint16_t sync;
};

class AudioAnalyzeLTC : public AudioStream
{
  public:
    AudioAnalyzeLTC(void) : AudioStream(1, inputQueueArray) {
      new_output = false;
    }

    bool available(void) {
      __disable_irq();
      bool flag = new_output;
      __enable_irq();
      return flag;
    }

    ltcframe_t read(void) {
      __disable_irq();
      ltcframe_t tempframe = ltcframe;
      new_output = false;
      __enable_irq();
      return tempframe;
    }

    int hour(ltcframe_t * ltc) {
      return 10 * ((int) (ltc->data >> 56) & 0x03)  + ((int) (ltc->data >> 48) & 0x0f) ;
    }
    
    int minute(ltcframe_t * ltc) {
      return 10 * ((int) (ltc->data >> 40) & 0x03)  + ((int) (ltc->data >> 32) & 0x0f) ;
    }
    
    int second(ltcframe_t * ltc) {
      return 10 * ((int) (ltc->data >> 24) & 0x03)  + ((int) (ltc->data >> 16) & 0x0f) ;
    }
    
    int frame(ltcframe_t * ltc) {
      return 10 * ((int) (ltc->data >>  8) & 0x03)  + ((int) (ltc->data >>  0) & 0x0f) ;
    }
    
    inline void decodeBitstream(unsigned newbit);
    virtual void update(void);

  private:
    audio_block_t *inputQueueArray[1];
    volatile bool new_output;
    int avsample;
    ltcframe_t ltcframe;
};

#endif