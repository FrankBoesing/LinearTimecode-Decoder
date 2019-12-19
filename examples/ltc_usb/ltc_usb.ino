/* Linear Timecode for Audio Library for Teensy 3.x / 4.x

   USB-Audio Example

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

/*

 https://forum.pjrc.com/threads/41584-Audio-Library-for-Linear-Timecode-(LTC)

 LTC example audio at: https://www.youtube.com/watch?v=uzje8fDyrgg

 Howto: Set USB-Type to USB-Audio, on PC: Play LTC Audio and set autio-output to use Teensy as audio-device.
 
*/

#include <Audio.h>
#include <analyze_ltc.h>

// GUItool: begin automatically generated code
AudioInputUSB            usb1;           //xy=386,450
AudioAnalyzeLTC          ltc1;         //xy=556,396
AudioOutputI2S           i2s1;           //xy=556,469
AudioConnection          patchCord1(usb1, 0, i2s1, 0);
AudioConnection          patchCord2(usb1, 0, ltc1, 0);
AudioConnection          patchCord3(usb1, 1, i2s1, 1);
// GUItool: end automatically generated code

ltcframe_t ltcframe;

void setup() {
  AudioMemory(12);
}

void loop() {
  if (ltc1.available()) {
    ltcframe = ltc1.read();
    Serial.printf("%02d:%02d:%02d.%02d\n", ltc1.hour(&ltcframe), ltc1.minute(&ltcframe), ltc1.second(&ltcframe), ltc1.frame(&ltcframe));  
  }
}
