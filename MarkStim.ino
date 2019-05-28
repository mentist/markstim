/*
Version: 2013-09-23~2018-01-09
Author: Yong-Jun Lin

References:
Definition of a TTL signal
  http://digital.natinst.com/public.nsf/$CXIV/ATTACH-AEEE-89LM9U/$FILE/TTL%20Specification.gif

History:
2013-09-23
 1. Teensy 2.0 may be better than Teensy++ 2.0 in that it is smaller.
2018-01-06
 1. This demo is modified from the official LED blink demo (File\Examples\Teensy\Tutorial1\Blink)
 2. This demo simulates TTL signals driving TMS pulses. Just use the LED pin as the positive pin.
2018-01-07
 1. Only consider Teensy 2 but not 3 because TTL relies on 5V but not 3V logic
 2. In Brain Products Recorder_Release_Notes_1-21-0201.pdf, the release notes of Version 1.20.0801 (December 2014) states that the minimum length of trigger signal is 2 ms for 1000 Hz and 0.8 ms for 2500 Hz recording. So choose 3 ms for now.
2018-01-08
 1. Added #if defined() and #elif defined() macros to generalize to applicable environments (Teensy 2.0 and Teensy++ 2.0, not yet Arduino)
2018-01-09
 1. Tested Magstim Super Rapid 2 Plus 1 and found that 3 ms TTL can reliably trigger TMS pulse.

Future:
 1. Test if 1 ms TTL can trigger TMS pulses
 2. Test the shortest TTL that can trigger TMS pulses
 3. Test the TMS response time offset from TTL onset with an oscilloscope


Copyright (C) 2013-2019  Yong-Jun Lin
This file is part of MarkStim, a TMS trigger/EEG event registration 
device. See <https://yongjunlin.com/MarkStim/> for the documentation 
and details.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
    
// Constants about hardware level wiring
#if defined(__AVR_ATmega32U4__) // Teensy 2.0
const int pinLED = 11;
#elif defined(__AVR_AT90USB1286__)  // Teensy++ 2.0
const int pinLED = 6;
#endif
#define LED_ON HIGH
#define LED_OFF LOW

void setup()
{
  // Set pin mode
  pinMode(pinLED, OUTPUT);  
}

void loop()
{
  digitalWrite(pinLED, LED_ON);
  delay(3); // 3ms
  digitalWrite(pinLED, LED_OFF);
  delay(4000);
}

