/*
Version: 2013-09-23~2018-01-15
Author: Yong-Jun Lin

References:
Definition of a TTL signal
  http://digital.natinst.com/public.nsf/$CXIV/ATTACH-AEEE-89LM9U/$FILE/TTL%20Specification.gif
On Teensyduino, Serial.begin() actually accesses USB (12 Mbit/sec) so the baud rate does not matter.
  https://www.pjrc.com/teensy/td_serial.html
On Teensyduino, Serial1.begin() accesses hardware serial UART. This is not useful for the current project.
  https://www.pjrc.com/teensy/td_uart.html
Serial.send_now() and transmit buffering
  https://www.pjrc.com/teensy/td_serial.html#txbuffer
Serial communication
 http://arduino.cc/en/Tutorial/ReadASCIIString
 http://arduinobasics.blogspot.com/2012/07/arduino-basics-simple-arduino-serial.html
 http://playground.arduino.cc/interfacing/python
 http://arduino.cc/en/Tutorial/SerialCallResponse
 Getting string:
 http://forum.arduino.cc/index.php/topic,45629.0.html
 http://forum.arduino.cc/index.php?topic=41888.0
Serial communication latency
 http://neophob.com/2011/04/serial-latency-teensy-vs-arduino/

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
 2. Set pin mode as INPUT_PULLUP so that the default is HIGH (5V). When the switch is on or when the button is pressed, it goes to LOW (0V; GND).
 3. Set the serial communication speed (can be arbitrary for Teensy, just use 57600 so that it is also a reasonable number for Arduino)
2018-01-10
 1. Branched out to develop a high-level protocol based on serial communication between the board and a computer.
 2. Controlled LED by serial communication.
 3. Reduced the test case to character echoing.
 4. Instead of echoing, respond by ++ to an incoming character.
 5. Tested whether CR+LF will be transmitted as part of the string.
2018-01-12
 1. ﻿Handshake, parsing command, and executing command (old syntax since 2013)
 2. Redesigned communication protocol
2018-01-15
 1. Updated the communication steps with the following hierarchy:
    Setup+Loop(Handshake+RealDeal(SaveToBuffer+PerformSettings(ResetBuffer)+Perform command(ResetBuffer)+ResetDevice))

Future:
 1. Test if 1 ms TTL can trigger TMS pulses
 2. Test the shortest TTL that can trigger TMS pulses
 3. Test the TMS response time offset from TTL onset with an oscilloscope
 4. Explore hardware vs. software serial speed
 5. Examine packet round trip time. Better to measure it as a function of string length.
 6. Test if Serial.send_now() matters for temporal precision! It might only matter for response pads.


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


// Constants for debugging
//#define DEBUGGING

// Constants for memory management
#define BUFSIZE 1025

// Constants about hardware level wiring
#if defined(__AVR_ATmega32U4__) // Teensy 2.0
const unsigned int baudRate = 57600;
const int pinLED = 11;
#elif defined(__AVR_AT90USB1286__)  // Teensy++ 2.0
const unsigned int baudRate = 57600;
const int pinLED = 6;
#endif
#define LED_ON HIGH
#define LED_OFF LOW

// Constants for communication
#define HANDSHAKE_CHAR '!'

// State variable and constants of the machine
/*
From Protocol.txt:
  state description
  10    waiting for handshake
  11    received !
  12    sent 'Teensy ready'
  20    (handshake done)-> listening
  31    heard settings
  35    doing settings
  41    heard command
  45    doing task ()
  51    heard reset
*/
unsigned char state = 10;

// String buffer for serial communication
char buffer[BUFSIZE] = "\0";
unsigned int pBuffer = 0;

// Settings variables
int nPeriods = 0;

// Command variables
int periodOn_ms = 0;
int periodOff_ms = 0;

void setup()
{
  // Initialize serial communication (actually USB if using Teensy)
  Serial.begin(baudRate);
  // Set pin mode
  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LED_ON);
  return;
}

void loop()
{
  char newByte = '\0';
  if (Serial.available())
  {
    newByte = Serial.read();  // One byte at a time
#ifdef DEBUGGING
    Serial.println("Received new byte.");  // Received new byte
#endif
    if (state == 10)
      Handshake(newByte);
    else  //>= 20
      RealDeal(newByte);
  }
  return;
}

void Handshake(char newByte)
{
  // Wait for '!' and reply with 'Teensy ready\n'

  if (newByte == HANDSHAKE_CHAR)
  {
    state = 11;
    //Serial.flush();
    Serial.println("Teensy ready");
    state = 12;
    memset(buffer, '\0', BUFSIZE);
    state = 20;
    digitalWrite(pinLED, LED_OFF);  // LED as state indicator (off means handshake is done)
  }
  return;
}

void RealDeal(char newByte)
{
  //Syntax:
  //1. Settings: <{param1},{param2},{param3},...>
  //2. Command: [{param1},{param2},{param3},...]
  //3. Reset: `
  //(\r and \n are not necessary and will be ignored)

  if (newByte == '`')
  {
    state = 51;
    Serial.println("Reset device.");
    ResetDevice();
    return;
  }

  if (state == 20)
  {
    if (newByte == '<')
      state = 31;
    else if (newByte == '[')
      state = 41;
  }
  else if (state == 31)
  {
    if (newByte != '>')
      SaveToBuffer(newByte);
    else
    {
      state = 35;
      PerformSettings();
    }
  }
  else if (state == 41)
  {
    if (newByte != ']')
      SaveToBuffer(newByte);
    else
    {
      state = 45;
      PerformCommand();
    }
  }
  return;
}

void SaveToBuffer(char newByte)
{
  buffer[pBuffer] = char(newByte);
  pBuffer++;
#ifdef DEBUGGING
  if (pBuffer == BUFSIZE)
  {
    Serial.println("Error: Input parameter longer than buffer size.");
    //Serial.flush();
  }
  else
    Serial.println("Writing new byte to buffer");
#endif
  return;
}

void PerformSettings()
{
  //Input: global variable buffer

  //Parse buffer
  nPeriods = atoi(strtok(buffer, ","));
#ifdef DEBUGGING
  Serial.println("Parsed settings.");
#endif
  ResetBuffer();

  //Execute settings
//#ifdef DEBUGGING
  Serial.println("Executed settings.");
//#endif
  state = 20;
  return;
}

void PerformCommand()
{
  //Input: global variable buffer

  //Parse buffer
  periodOn_ms = atoi(strtok(buffer, ","));
  periodOff_ms = atoi(strtok(NULL, ","));
#ifdef DEBUGGING
  Serial.println("Parsed command.");
#endif
  ResetBuffer();

  //Execute command
  for (int i = 0; i < nPeriods; i++)
  {
    digitalWrite(pinLED, LED_ON);
    delay(periodOn_ms);
    digitalWrite(pinLED, LED_OFF);
    delay(periodOff_ms);
  }
#ifdef DEBUGGING
  Serial.println("Executed command.");
#endif
  state = 20;

  return;
}

void ResetBuffer()
{
  pBuffer = 0;
  memset(buffer, '\0', BUFSIZE);
  Serial.flush();
#ifdef DEBUGGING
  Serial.println("Reset buffer.");
#endif
  return;
}

void ResetDevice()
{
  //To be implemented
  return;
}

