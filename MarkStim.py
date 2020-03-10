# Date: 2020-03-05~2020-03-09
# Author: Yong-Jun Lin
# Purpose: A wrapper class for communicating with the MarkStim TMS trigger/EEG event registration device
# 
# Copyright (C) 2013-2020  Yong-Jun Lin
# This file is part of MarkStim, a TMS trigger/EEG event registration 
# device. See <https://yongjunlin.com/MarkStim/> for the documentation 
# and details.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import sys, time, serial

class MarkStim(object):
    def __init__(self, port):
        self.hS = serial.Serial(port=port, baudrate=57600, timeout=None, writeTimeout=None)

    def handshake(self):
        self.HandshakeProtocol(self.hS, 'MarkStim')
    def setup(self, bTTL=True, TTLPulseWidth=1000):
        data = '<%d,%ld>' % (bTTL, TTLPulseWidth)
        if (sys.version_info > (3, 0)):
            # https://stackoverflow.com/questions/14454957/pyserial-formatting-bytes-over-127-return-as-2-bytes-rather-then-one
            #data = data.encode('ascii')  # does not work in Python 3 anymore because it only accepts range 0~127
            data = data.encode('latin_1')
        self.hS.write(data)
    def trigger(self, num):
        if num < 0 or num > 255:
            raise(Exception('MarkStim.trigger() accepts integers ranging from 0 to 255.'))
        else:
            data = '['+chr(num)+']'
            if (sys.version_info > (3, 0)):
                # https://stackoverflow.com/questions/14454957/pyserial-formatting-bytes-over-127-return-as-2-bytes-rather-then-one
                #data = data.encode('ascii')  # does not work in Python 3 anymore because it only accepts range 0~127
                data = data.encode('latin_1')
            self.hS.write(data)
    def reset(self):
        self.ResetProtocol(self.hS)
    
    @classmethod
    def help(cls):
        print('.handshake() to start talking to the device\n.setup() to setup (normally not required).\n.trigger(num) to trigger\n.reset() to reset the device')
    
    @staticmethod
    def HandshakeProtocol(serialDevice, deviceName, handshakeTimeout=0.05, tDelay=2, symbol='!', feedbackStr='Teensy ready'):
        if (sys.version_info > (3, 0)):
            symbol = symbol.encode()
        serialDevice.flushInput()
        serialDevice.flushOutput()
        time.sleep(tDelay)
        t0 = time.time()
        while True:
            # Keep sending symbol (typically "!" or "\n") to probe whether Teensy or Arduino is ready for serial communication
            # https://stackoverflow.com/questions/35642855/python3-pyserial-typeerror-unicode-strings-are-not-supported-please-encode-to
            serialDevice.write(symbol)
            if serialDevice.inWaiting():
                feedback = serialDevice.readline()
                # https://problemsolvingwithpython.com/11-Python-and-External-Hardware/11.02-Bytes-and-Unicode-Strings/
                if (sys.version_info > (3, 0)):
                    feedback = feedback.decode()
                if feedback in [feedbackStr, "Teensy ready\r\n", "Arduino ready\r\n", "Serial input device ready\r\n", "Serial output device ready\r\n"]:
                    print(("Establishing serial communication with %s took %.4f sec" % (deviceName, time.time()-t0)))
                    break
            else:
                if time.time()-t0 > handshakeTimeout:
                    raise Exception("Failed establishing serial communication with %s (took longer than %.2f sec)" % (deviceName, handshakeTimeout))
    @staticmethod
    def ResetProtocol(serialDevice, symbol="`"):
        if (sys.version_info > (3, 0)):
            symbol = symbol.encode()
        try:   # in case that this function is called multiple times
            serialDevice.flushInput()
            serialDevice.flushOutput()
            serialDevice.write(symbol)   # My reserved character for resetting Teensy by software. They do not matter for Arduino because Arduino resets automatically when a new serial connection is initiated.
            time.sleep(0.1)  # Sometimes the reset does not work if write() is followed by immediate close()
            serialDevice.close() # Some serial devices are both for input and output, depending on the device's state. So it is possible to close() twice.
        except:
            pass

        
if __name__ == "__main__":
    #MarkStim.help()
    
    # Define the port
    if sys.platform == 'darwin':    # Mac
        port = '/dev/tty.usbmodem12341'
    elif sys.platform.startswith('linux'):  # Linux
        port = '/dev/ttyACM0'
    elif sys.platform == 'win32':   # Windows
        port = 'COM5'
    # Initialize the connection
    mkst = MarkStim(port)
    # At the beginning of your experiment script, handshake with the MarkStim device. The orange LED should be OFF after a successful handshake. Handshaking would not work if the LED was not ON in the beginning.
    mkst.handshake()
    # Normally, the setup line is not requires. These are the default values.
    mkst.setup(True, 1000)

    # The trigger line should follow your visual presentation, sound onset, etc.  Inside your task loop, call this function as many times as you need with trigger codes (ranging from 0 to 255) defined by you.
    mkst.trigger(255)   # accepts integers ranging from 0 to 255

    # After the experiment is over, reset the device so that it goes back to idle mode (when orange LED is ON).
    mkst.reset()
