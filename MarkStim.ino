/*
Version: 2013-09-23~2019-07-07
Author: Yong-Jun Lin

History:
2013-09-23
 1. Teensy 2.0 may be better than Teensy++ 2.0 in that it is smaller.
2014-05-13
 1. Implemented a wrapper function of _restart_Teensyduino_().
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
2018-01-16
 1. Combined the new serial communication syntax code with the button/switch code so that there is a reset button for easy benchmarking. Fleshed out ResetDevice().
 2. Test the TTL signals generated by a shift register with an oscillope or an EEG recorder.
 3. Incorporated shift register output.
2018-01-17
 1. Validated waveform by oscilloscope for each shift register outpun pin.
 2. Wrote SendTrigger() as a wrapper of update8BitShiftRegister().
 3. Discovered by an oscilloscope that the TTL Pulse Width cannot be smaller than 38 ms. No matter delay() or delayMicroseconds() is used and no matter the input value, the delay was constantly 38 ms for Teensy 2.0 and this program.
 4. Changed reset state ID. Added demo state ID.
 5. Defined function CheckReset() and CheckDemoSwitch().
 6. Considered the condition for restarting demo and the condition for stopping demo (back to waiting for handshake). Turns off LED when entering demo mode and turns it back on when leaving it.
 7. Wrote and put LED feedback code inside DEBUGGING mode.
 8. Allowed >16 ms TTL pulse width.
 9. Encoded the trigger value by char rather than by a string of number.
2018-02-02
 1. Prepended 2^i for 4 rounds before 0~255 in the demo trigger mode.
2018-02-05
 1. Fixed the wrong syntax 2^i to pow(2^7)
2019-07-06
 1. State 51 can only be reached under state 20 now so that chr(96)='`' can be a legitimate value.
2019-07-07
1. Trigger value can take any value including chr(93)=']' now.

Future:
 1. Test if 1 ms TTL can trigger TMS pulses
 2. Test the shortest TTL that can trigger TMS pulses
 3. Test the TMS response time offset from TTL onset with an oscilloscope
 4. Explore hardware vs. software serial speed
 5. Examine packet round trip time. Better to measure it as a function of string length.
 6. Test if Serial.send_now() matters for temporal precision! It might only matter for response pads.

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
#define LED_FEEDBACK_DUR 1  // (ms)
//#define SCAFFOLDING

// Constants for memory management
#define BUFSIZE 1025

// Constants about hardware level wiring
#if defined(__AVR_ATmega32U4__) // Teensy 2.0
const unsigned int baudRate = 57600;
const int pinLED = 11;
const int pinSwitch1 = 10;
const int pinSwitch2 = 9;
const int pinResetButton = 6;
const unsigned int pinData = 12;  //A10
const unsigned int pinClock = 13;  //A9
const unsigned int pinLatch = 14;  //A8
#elif defined(__AVR_AT90USB1286__)  // Teensy++ 2.0
const unsigned int baudRate = 57600;
const int pinLED = 6;
const unsigned int pinData = 45;  //F7
const unsigned int pinClock = 44;  //F6
const unsigned int pinLatch = 43;  //F5
#endif
#define LED_ON HIGH
#define LED_OFF LOW

// Constants for communication
#define HANDSHAKE_CHAR '!'
#define SETTINGS_OK_CHAR '^'
#define SETTINGS_NOTOK_CHAR '%'
#define DELAYOFDELAY 38 // Not sure whether this is board or program specific. At least the oscillocope shows the same delay for delay() and delayMicroseconds().

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
  43    heard more command
  45    doing task ()
  71    heard demo
  91    heard reset
*/
unsigned char state = 10;

// String buffer for serial communication
char buffer[BUFSIZE] = "\0";
unsigned int pBuffer = 0;

// Settings variables (with default values)
int bTTL = true; // bool actually
long TTLPulseWidth = 1000-DELAYOFDELAY;  // (microsec)

// Command variables
byte triggerVal = 0;


void CheckDemoSwitch()
{
  bool bSwitch1 = digitalRead(pinSwitch1);
  if (bSwitch1 == LOW)
  {
    state = 71;
#ifdef DEBUGGING
    Serial.println("Shift register demo (cyclic 0~255).");
#endif
    Demo();
  }
}

void update8BitShiftRegister(byte val)
{
  digitalWrite(pinLatch, LOW);     //Pulls the chips latch low
  shiftOut(pinData, pinClock, MSBFIRST, val); //Shifts out the 8 bits to the shift register
  digitalWrite(pinLatch, HIGH);   //Pulls the latch high displaying the data
#ifdef DEBUGGING
  bool bSwitch2 = digitalRead(pinSwitch2);
  if (bSwitch2 == LOW)
  {
    digitalWrite(pinLED, LED_ON);
    delay(LED_FEEDBACK_DUR); // 1 ms is sufficient for visible LED feedback upon trigger onset
    digitalWrite(pinLED, LED_OFF);
  }
#endif
}

void ResetDevice()
{
  // The reset button on Teensy is for reset to bootloader, not for power-on reset.
  // This function emulates power-on reset by software, so reset no longer requires unplugging and replugging the USB cable.
  // Finalize serial communication
  Serial.end();  // Must end() before begin()
  // Reset shift register
  update8BitShiftRegister(0);
  // Reset (emulate Arduino style reset)
  _restart_Teensyduino_();
  return;
}

void CheckReset()
{
  bool bResetButton = digitalRead(pinResetButton);
  if (bResetButton == LOW)
  {
    state = 91;
#ifdef DEBUGGING
    Serial.println("Reset device.");
#endif
    ResetDevice();
  }
  return;
}

void Demo()
{
  bool bSwitch1;
  digitalWrite(pinLED, LED_OFF);
  // 2^0~2^7 for 4 rounds
  for (int j = 0; j < 4; j++)
  {
    delay(250); // (ms)
    triggerVal = 1;
    update8BitShiftRegister(triggerVal);
    delay(250); // (ms)
    triggerVal = 2;
    update8BitShiftRegister(triggerVal);
    delay(250); // (ms)
    triggerVal = 4;
    update8BitShiftRegister(triggerVal);
    delay(250); // (ms)
    triggerVal = 8;
    update8BitShiftRegister(triggerVal);
    delay(250); // (ms)
    triggerVal = 16;
    update8BitShiftRegister(triggerVal);
    delay(250); // (ms)
    triggerVal = 32;
    update8BitShiftRegister(triggerVal);
    delay(250); // (ms)
    triggerVal = 64;
    update8BitShiftRegister(triggerVal);
    delay(250); // (ms)
    triggerVal = 128;
    update8BitShiftRegister(triggerVal);
    /*
    for (int i = 0; i < 8; i++)
    {
      delay(250); // (ms)
      triggerVal = pow(2, i);
      update8BitShiftRegister(triggerVal);
    */
  }
  // 0~255 for 1 round
  for (triggerVal = 0; triggerVal < 256; triggerVal++)
  {
    delay(250); // (ms)
    update8BitShiftRegister(triggerVal);
    //So the Q0 (2^0) pin will have square waves at 4 Hz
    //So the Q1 (2^1) pin will have square waves at 2 Hz
    //So the Q2 (2^2) pin will have square waves at 1 Hz
    //So the Q3 (2^3) pin will have square waves at 0.5 Hz
    //So the Q4 (2^4) pin will have square waves at 0.25 Hz
    //So the Q5 (2^5) pin will have square waves at 0.125 Hz
    //So the Q6 (2^5) pin will have square waves at 0.0625 Hz
    //So the Q7 (2^5) pin will have square waves at 0.03125 Hz
    //Use an oscilloscope to verify
    //Or use a EEG trigger monitor to view if the trigger values are in cycles of 0~255 (usually 0 is not recorded, indiciating no events at all)

    //Condition for restarting demo
    CheckReset();

    //Condition for exiting demo
    bSwitch1 = digitalRead(pinSwitch1);
    if (bSwitch1 == HIGH)
    {
      digitalWrite(pinLED, LED_ON);
      state = 10;
      break;
    }
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


  if (state == 20)
  {
    if (newByte == '<')
      state = 31;
    else if (newByte == '[')
      state = 41;
    else if (newByte == '`')
    {
      state = 51;
      Serial.println("Reset device.");
      ResetDevice();
    }
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
    state = 43;
    SaveToBuffer(newByte);
  }
  else if (state == 43)
  {
    // The ending byte is ']' in the previous version, but it can actually be omitted.
    if (newByte == ']')
    {
      state = 45;
      PerformCommand();
    }
    else
    {
#ifdef DEBUGGING
      Serial.println("Unexpected command format.");
#endif
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
  bTTL = atoi(strtok(buffer, ",")); // TTL (goes back to 0 as background default value) or value locking mode
  TTLPulseWidth = atol(strtok(NULL, ","));
  if (TTLPulseWidth <= DELAYOFDELAY)  // (microsec)
  {
    //TTLPulseWidth = TTLPulseWidth;  // Beyond correction if <= DELAYOFDELAY
    Serial.print(SETTINGS_NOTOK_CHAR);  // Feedback character
  }
  else
  {
    TTLPulseWidth = TTLPulseWidth-DELAYOFDELAY;
    Serial.print(SETTINGS_OK_CHAR);  // Feedback character
  }
#ifdef DEBUGGING
  Serial.println("Parsed settings.");
  Serial.printf("<%d,%d>", bTTL, TTLPulseWidth);
#endif
  ResetBuffer();

  //Execute settings
#ifdef DEBUGGING
  Serial.println("Executed settings.");
#endif
  state = 20;
  return;
}

void PerformCommand()
{
  //Input: global variable buffer

  //Parse buffer
#ifdef SCAFFOLDING
  triggerVal = atoi(strtok(buffer, ",")); // For debugging! For example, "[65]".
#else
  triggerVal = char(buffer[0]); // The real 1 byte coding. For example, For example, "[A]".
#endif
#ifdef DEBUGGING
  Serial.println("Parsed command.");
  Serial.println(triggerVal);
#endif
  ResetBuffer();

  //Execute command
  SendTrigger(triggerVal);
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

void SendTrigger(byte val)
{
  update8BitShiftRegister(val);
  // From Brain Products: Recorder_Release_Notes_1-20-0801.pdf
  // Minimal length of trigger signal under 1000 Hz is 2 ms
  if (bTTL)
  {
    //https://www.arduino.cc/reference/en/language/variables/data-types/int/
    //https://www.arduino.cc/reference/en/language/functions/time/delaymicroseconds/
    if (TTLPulseWidth <= 32767)
      delayMicroseconds(TTLPulseWidth); // (microsec) This command itself has about 38 microsec delay. The same thing applies to delay().
    else
      delay(TTLPulseWidth/1000); // (ms)
    update8BitShiftRegister(0);
  }
}

void setup()
{
  // Initialize serial communication (actually USB if using Teensy)
  Serial.begin(baudRate);
  // Set pin mode
  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LED_ON);
  pinMode(pinSwitch1, INPUT_PULLUP);
  pinMode(pinSwitch2, INPUT_PULLUP);
  pinMode(pinResetButton, INPUT_PULLUP);
  pinMode(pinData, OUTPUT);
  pinMode(pinClock, OUTPUT);
  pinMode(pinLatch, OUTPUT);
  // Reset shift register
  update8BitShiftRegister(0);
  return;
}

void loop()
{
  CheckDemoSwitch();
  CheckReset();

  char newByte = '\0';
  if (Serial.available())
  {
    newByte = Serial.read();  // One byte at a time
#ifdef DEBUGGING
    Serial.println("Received a new byte.");
#endif
    if (state == 10)
      Handshake(newByte);
    else  //>= 20
      RealDeal(newByte);
  }
  return;
}
