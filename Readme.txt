Copyright (C) 2013-2020  Yong-Jun Lin
This file is part of MarkStim, a TMS triggering/EEG event registering
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

<What's in the package?>
1. Pictorial circuit diagrams
   See Wiring/*.png
	 (Note that this device is based on Teensy, not Arduino for accurate timing.
	 See also: http://neophob.com/2011/04/serial-latency-teensy-vs-arduino/)
2. Communication protocol (based on serial communication)
   Protocol.txt
3. Teensy code
   MarkStim.ino

4. Python code for controlling the MarkStim device
   MarkStim.py
5. MATLAB Mex code for controlling the MarkStim device
   Mex/*
	 		MarkStim.c					C source code of the MATLAB mex file
	 	  MarkStim.m					The help file
			MarkStim.mexa64			Compiled mex file for Linux 64-bit
			MarkStim.mexmaci		Compiled mex file for Intel-based Mac 32-bit
			MarkStim.mexmaci64	Compiled mex file for Intel-based Mac 64-bit

6. Benchmarking the timing performance via Python
   Benchmark.py
	 (Excellence timing performance with Python)
7. Benchmarking the timing performance via MATLAB Mex
   BenchmarkThroughMex.m
	 (Excellence timing performance with MATLAB Mex)
8. Benchmarking the timing performance via MATLAB
   Benchmark.m
	 (Terrible timing performance  because serial communication is slow through
	 Java runtime environment. MATLAB users should use the mex file to control
	 MarkStim.)

<Python users>
Grab the MarkStim.py file

<MATLAB users>
Grab a MarkStim.mex* file for your platform along with its help file MarkStim.m
If the mex file does not work, build from the C MEX file with
  mex MarkStim.c
In MATLAB, type
	help MarkStim
to find out the command syntax.
