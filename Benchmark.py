"""
Version: 2013-09-19~2013-09-19
Author: Yong-Jun Lin
Purpose: Measure the serial communication round-trip between the computer and a board
History:
Future:
References:
 http://neophob.com/2010/11/arduino-serial-latency/
 http://neophob.com/2011/04/serial-latency-teensy-vs-arduino/


Copyright (C) 2013-2020  Yong-Jun Lin
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
"""
# Dependencies:
import time, serial, numpy
import matplotlib.pyplot as pyplot


# Hardware settings
#port = '/dev/tty.usbmodem411'  # Arduino Uno
port = '/dev/tty.usbmodem14321' # Teensy++ 2.0
baudRate = 57600
#baudRate = 115200  # Large variance
# http://pyserial.sourceforge.net/pyserial_api.html
#timeOut_read = 0.05   # (sec)
#timeOut_write = 0.05
# Blocking mode: Wait forever
timeOut_read = None   # (sec)
timeOut_write = None
# Blocking mode: Non-blocking
#timeOut_read = 0   # (sec)
#timeOut_write = 0

# Declare state variables
READING = False
WRITING = True
state = WRITING # Opposite to Arduino's initial state which is READING, the Python script starts with WRITING.
bDebug = False

# For data gathering and plotting
nTrials = 1000
x = range(1, nTrials+1)
yRead = []
yWrite = []

# Initialize Arduino
arduino = serial.Serial(port=port, baudrate=baudRate, timeout=timeOut_read, writeTimeout=timeOut_write)

# While loop
# The first round trip time indicates how long it takes Arduino to be ready for serial communication
t0 = time.time()
n = 1
while True:
    if state:
        t0_write = time.time()
        arduino.write(".")
        timeWrite = time.time()-t0_write
        yWrite.append(timeWrite)
        if bDebug:
            print ("Trial %d: writing took %f sec" % (n, timeWrite))
        state = READING
        t0_read = time.time()
    else:
        feedback = arduino.read()  # Ignore the content of feedback for now
        if len(feedback) > 0:
            timeRead = time.time()-t0_read
            yRead.append(timeRead)
            if bDebug:
                print ("Trial %d: reading '%s' took %f sec" % (n, feedback, timeRead))
            n += 1  # Each increment means write and read
            state = WRITING
    if n > nTrials:
        break
print ("%d trials experiment took %f sec" % (nTrials, time.time()-t0))

fig1 = pyplot.figure()
# http://matplotlib.org/examples/lines_bars_and_markers/line_demo_dash_control.html
lineRead, = pyplot.plot(x, yRead, 'r-')
lineWrite, = pyplot.plot(x, yWrite, 'b-')
pyplot.xlim(x[0], x[-1])
pyplot.ylim(0, 0.01)
# http://matplotlib.org/users/legend_guide.html
pyplot.legend([lineRead, lineWrite], ["Read", "Write"])
pyplot.xlabel('Trial number')
pyplot.ylabel('(ms)')
pyplot.title('Serial communication latency')
pyplot.show()

# Conclusion:
# 1. Roundtrip time from computer to Arduino Uno is about 4ms. This is consistent with a previous study (http://neophob.com/2010/11/arduino-serial-latency/).
# 2. Get Teensy (http://www.pjrc.com/store/teensy_pins.html) or try something else.
