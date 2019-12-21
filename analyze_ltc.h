/* Audio Library for Teensy 3.X
   Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com

   This Code:
   Copyright (c) 2019, Frank Bösing, f.boesing (at) gmx.de

   Please support PJRC's efforts to develop open source software by purchasing
   Teensy or other PJRC products

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
  uint32_t timestampfirstedge;
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


    //Auxiliary functions:

    int hour(ltcframe_t * ltc)   {
      return 10 * ((int) (ltc->data >> 56) & 0x03)  + ((int) (ltc->data >> 48) & 0x0f);
    }
    int minute(ltcframe_t * ltc) {
      return 10 * ((int) (ltc->data >> 40) & 0x07)  + ((int) (ltc->data >> 32) & 0x0f);
    }
    int second(ltcframe_t * ltc) {
      return 10 * ((int) (ltc->data >> 24) & 0x07)  + ((int) (ltc->data >> 16) & 0x0f);
    }
    int frame(ltcframe_t * ltc)  {
      return 10 * ((int) (ltc->data >>  8) & 0x03)  + ((int) (ltc->data >>  0) & 0x0f);
    }
    bool bit10(ltcframe_t * ltc) {
      return (int) (ltc->data >> 10) & 0x01;
    }
    bool bit11(ltcframe_t * ltc) {
      return (int) (ltc->data >> 11) & 0x01;
    }
    bool bit27(ltcframe_t * ltc) {
      return (int) (ltc->data >> 27) & 0x01;
    }
    bool bit43(ltcframe_t * ltc) {
      return (int) (ltc->data >> 43) & 0x01;
    }
    bool bit58(ltcframe_t * ltc) {
      return (int) (ltc->data >> 58) & 0x01;
    }
    bool bit59(ltcframe_t * ltc) {
      return (int) (ltc->data >> 59) & 0x01;
    }

    uint32_t userdata(ltcframe_t * ltc) {
      return  ((int) (ltc->data >>  4) & 0x0000000fUL) | ((int) (ltc->data >>  8) & 0x000000f0UL) |
              ((int) (ltc->data >> 12) & 0x00000f00UL) | ((int) (ltc->data >> 16) & 0x0000f000UL) |
              ((int) (ltc->data >> 20) & 0x000f0000UL) | ((int) (ltc->data >> 24) & 0x00f00000UL) |
              ((int) (ltc->data >> 28) & 0x0f000000UL) | ((int) (ltc->data >> 32) & 0xf0000000UL);
    }

    virtual void update(void);

  private:
    audio_block_t *inputQueueArray[1];
    volatile bool new_output;
    int avsample;
    ltcframe_t ltcframe;
    inline void decodeBitstream(unsigned newbit);
};

#endif