/*
Version: 2018-01-11~2020-03-09
Author: Yong-Jun Lin

History:
2018-01-11 YJL	Modified standard serial communication code in POSIX C into a mex function.
				Implemented input variable extraction
2018-01-12 YJL	Implemented persistent variable
2018-01-15 YJL	Implemented serial C
2018-01-16 YJL	Added second order commands 'i(nitialize)', '(s)ettings', '(t)rigger', 'e(x)it' to avoid low level function calls and simplifiy Matlab code complexity. This version is for triggering. Adapted from TeensyBenchmark.c
2018-01-17 YJL	Made the variable TTLPulseWidth long instead of int
2019-08-14 YJL	Solved the problem on Linux where the handshake string drops \r.

Future:
1. Add time out error for handshaking
2. Try clear mex to make sure that Matlab does not crash
3. Need to return the string

Steps and references:
1. Enable C compiler by installing Xcode
2. Enable Matlab Mex with C on Mac
	mex -setup
3. Compile a hello world mex file
	http://www.shawnlankton.com/2008/03/getting-started-with-mex-a-short-tutorial/
	https://www.mathworks.com/help/matlab/matlab_external/introducing-mex-files.html?s_tid=gn_loc_drop
	.mexmaci64
4. Compile a C or C++ hello world program in Xcode
	https://stackoverflow.com/questions/2603489/how-do-i-compile-a-c-file-on-my-mac
	https://www.macworld.co.uk/how-to/mac/learn-c-in-mac-os-x-3639920/
	Goes to ~/Library/Developer/Xcode/DerivedData/{ProjName-hash}/Build/Products/Debug/{ProjName}
5. Therefore, develop and debug by Xcode first and then modify the completed code into code for mex
6. At least 3 ways to do serial communication on Mac
	POSIX C (the simplest code)
		Google macos serial communication code c
		https://gist.github.com/chomy/3798582
		https://www.cmrr.umn.edu/~strupp/serial.html
		https://stackoverflow.com/questions/21019148/reading-from-serial-port-on-linux-and-osx
		https://github.com/yida/bus/blob/master/matlab/serialopen.c
		https://github.com/xanthium-enterprises/Serial-Port-Programming-on-Linux/blob/master/serial.c
		http://todbot.com/blog/2006/12/06/arduino-serial-c-code-to-talk-to-arduino/
		https://raw.githubusercontent.com/todbot/arduino-serial/master/arduino-serial.c
		https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
	Cocoa C
		http://forum.arduino.cc/index.php/topic,39516.0.html
		https://developer.apple.com/library/content/documentation/DeviceDrivers/Conceptual/WorkingWSerial/WWSerial_SerialDevs/SerialDevices.html
		http://www.harmless.de/cocoa-code.php#serialport
		IOKit
			https://developer.apple.com/library/content/samplecode/SerialPortSample/Listings/SerialPortSample_SerialPortSample_c.html#//apple_ref/doc/uid/DTS10000454-SerialPortSample_SerialPortSample_c-DontLinkElementID_4
			https://developer.apple.com/library/content/documentation/DeviceDrivers/Conceptual/WorkingWSerial/WWSerial_SerialDevs/SerialDevices.html
	Objective C
		https://stackoverflow.com/questions/6153818/objective-c-serial-mac-os-x
		http://blog.andrewmadsen.com/post/26512371699/orsserialport-a-new-objective-c-serial-port-library
	Linux C
		https://github.com/cbrake/linux-serial-test/blob/master/linux-serial-test.c
	Swift
7. Some serial communication examples in C on Windows
	Google Matlab serial communication mex
	https://www.mathworks.com/matlabcentral/fileexchange/62545-mex-c-serial-interface
	https://www.mathworks.com/matlabcentral/fileexchange/25478-serialio-mex
	https://www.mathworks.com/matlabcentral/fileexchange/31958-serialdatastream
	https://www.mathworks.com/matlabcentral/fileexchange/23780-serialwrite-mex
8. Choose C to carry out Serial communication
	where there is plenty resource from linux C as well
		Google linux c serial open close read write
		https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
		http://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html#AEN144
		https://www.codeproject.com/Questions/718340/C-program-to-Linux-Serial-port-read-write
9. Compile into mex to benchmark serial communication time
	https://www.mathworks.com/matlabcentral/answers/342045-using-c-code-for-serial-port-communication-in-matlab-via-mex-file
	But do not want to separate open, read, write, close into separate mex files
		Pass an additional string
			Need to learn how to pass and receive variables
	Also want to keep static variables in mex
10. Persistent memory in mex.
	https://www.mathworks.com/matlabcentral/answers/103891-how-can-i-make-memory-persistent-between-calls-to-a-mex-file-in-matlab
    https://www.mathworks.com/help/matlab/apiref/mexmakememorypersistent.html
    https://www.mathworks.com/help/matlab/apiref/mexmakearraypersistent.html
11. Memory management
    https://www.mathworks.com/help/matlab/apiref/mxfree.html
    https://www.mathworks.com/help/matlab/apiref/mxdestroyarray.html
    https://www.mathworks.com/help/matlab/matlab_external/memory-management.html


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
*/


//#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
//#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
//#include <sys/types.h>
//#include <sys/uio.h>
#include <getopt.h>
//#include <stdlib.h>
#include "mex.h"

// Constants for debugging
//#define DEBUGGING

// Constants for memory management
#define BUFSIZE 1025

static long *deviceID = NULL;
void exitFcn()
{
	if (deviceID != NULL)
		mxFree(deviceID);
}

int openPort (char *deviceNamePath)
{
	int fd; //File descriptor
	fd = open(deviceNamePath, O_RDWR | O_NOCTTY | O_NDELAY);	//O_RDWR: Read/Write access to serial port; O_NOCTTY: No terminal will control the process
	if (fd < 0)
		mexErrMsgTxt("Could not open serial port.");
	else
		mexPrintf("Port opened.\n");
	return fd;
}

void configurePort (int fd, long baudRate)
{
	struct termios options;

	tcgetattr(fd, &options);
	cfsetispeed(&options, B57600);
	cfsetospeed(&options, B57600);

	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag |= (CLOCAL | CREAD);
//	options.c_cc[VMIN] = 1;	//blocking until read

	//https://www.cmrr.umn.edu/~strupp/serial.html
	fcntl(fd, F_SETFL, 0);	//For reading blocking

	if (tcsetattr (fd, TCSANOW, &options) != 0)
	{
		close(fd);
		mexErrMsgTxt("Error setting tty attributes.");
	}

	if (tcgetattr(fd, &options) == -1)
	{
		close(fd);
		mexErrMsgTxt("Error getting tty attributes.");
	}

	mexPrintf("Port configured.\n");
	return;
}

void mexFunction(int nlhs,mxArray *plhs[],int nrhs, const mxArray *prhs[])
{
	/*
		Syntax:
		'o', deviceNamePath, baudRate, TTLwindowSize
		'r'
		'w',
		'c',
	*/
	char *subCmd, *strParamBuf;
	size_t strParamBufLen;
	long baudRate, TTLwindowSize;
	char buf[BUFSIZE] = {'\0'};
	int nBytesToRead = 0;
	int bTTL = 0;
	long TTLPulseWidth = 0;
	int triggerVal = 0;	// using char crashes. don't know why.
	int ret = 0;

	// check for proper number of arguments
	if (nrhs < 1)
		mexErrMsgTxt("At least one inputs required (string) for the subcommand: '(o)pen', '(r)ead', '(w)rite', or '(c)'lose.");
	else if (nlhs > 0)
		mexErrMsgTxt("This mex file does not support output arguments.");

	// First input must be a string
	if (mxIsChar(prhs[0]) != 1)
		mexErrMsgTxt("The first input argument must be a string.");

	if (deviceID == NULL)
	{
		/* since deviceID is initialized to NULL, we know
		this is the first call of the MEX-function
		after it was loaded.  Therefore, we should
		set up deviceID and the exit function. */
		/* Allocate array. Use mexMackMemoryPersistent to make the allocated memory persistent in subsequent calls*/
#ifdef DEBUGGING
			mexPrintf("First call to MEX-file\n");
#endif
		deviceID = mxCalloc(1, 8);	// Why 8, not 4? For double or for long?
		mexMakeMemoryPersistent(deviceID);
		mexAtExit(exitFcn);
	}

	// copy the string data from prhs[0] into a C string subCmd.
	subCmd = mxArrayToString(prhs[0]);

	if (strcmp(subCmd, "o") == 0 || strcmp(subCmd, "i") == 0)
	{
		// Argument processing
		if (nrhs < 4)
			TTLwindowSize = 3;	// (ms)
		else
			TTLwindowSize = (long)*mxGetPr(prhs[3]);	// copy the numeric data from prhs[3] into a C variable.
		if (nrhs < 3)
			baudRate = 57600;	// (bit/sec)
		else
			baudRate = (long)*mxGetPr(prhs[2]);	// copy the numeric data from prhs[2] into a C variable.
		if (nrhs < 2)
			mexErrMsgTxt("When the first parameter is 'o' or 'i', the second parameter must be a string of the device name path, such as '/dev/cu.usbmodem12341'.");
		else
			strParamBuf = mxArrayToString(prhs[1]);	// copy the string data from prhs[1] into a C string strParamBuf.

		// Serial communication
		//deviceID[0] = 999;	//Assign arbitrary number for debugging
		deviceID[0] = (long) openPort(strParamBuf);	// Open serial port
		configurePort((int) deviceID[0], baudRate);	// Configure serial port

		// Handshaking
		int ret = 0;
		if (strcmp(subCmd, "i") == 0)
		{
			// Serial communication
			ret = write(deviceID[0], "!", 1);
			if (ret < 0)
				mexErrMsgTxt("write() error during handshake");
#ifdef DEBUGGING
			else
				mexPrintf("Wrote %d byte(s)\n", ret);
#endif
			ret = read(deviceID[0], buf, strlen("Teensy ready")+2);
			if (ret < 0)
				mexErrMsgTxt("read() error during handshake");
#ifdef DEBUGGING
			else
				mexPrintf("Read %d byte(s)\n", ret);
#endif
#ifdef __linux__
			if (ret != strlen("Teensy ready")+1)
				mexErrMsgTxt("Number of bytes returned is wrong during handshake");
			if (strcmp(buf, "Teensy ready\n") == 0)
				mexPrintf("Successful handshake.\n");
#else
			if (ret != strlen("Teensy ready")+2)
				mexErrMsgTxt("Number of bytes returned is wrong during handshake");
			if (strcmp(buf, "Teensy ready\r\n") == 0)
				mexPrintf("Successful handshake.\n");
#endif
		}

		// Free memory
		mxFree(subCmd);
		mxFree(strParamBuf);
	}
	else if (strcmp(subCmd, "f") == 0)	//show file descriptor
	{
		mexPrintf("%d\n", deviceID[0]);

		// Free memory
		mxFree(subCmd);
	}
	else if (strcmp(subCmd, "s") == 0)	//settings
	{
		// Argument processing
		if (nrhs < 3)
			mexErrMsgTxt("When the first parameter is 's', the second and third parameters, bTTL and TTLPulseWidth, must be numbers.");
		else
		{
			bTTL = (int)*mxGetPr(prhs[1]);	// copy the numeric data from prhs[1] into a C variable.
			TTLPulseWidth = (long)*mxGetPr(prhs[2]);	// copy the numeric data from prhs[2] into a C variable.
		}
		sprintf(buf, "<%d,%ld>", bTTL, TTLPulseWidth);
		mexPrintf("%s\n", buf);

		// Serial communication
		ret = write(deviceID[0], buf, strlen(buf));
#ifdef DEBUGGING
		mexPrintf("Wrote %d byte(s).\n", z);
#endif
		ret = read(deviceID[0], buf, 1);

		// Free memory
		memset(buf, '\0', BUFSIZE);
		mxFree(subCmd);
	}
	else if (strcmp(subCmd, "t") == 0)	//trigger
	{
		// Argument processing
		if (nrhs < 2)
			mexErrMsgTxt("When the first parameter is 't', the second parameter must be a number.");
		else
			triggerVal = (int)*mxGetPr(prhs[1]);	// copy the numeric data from prhs[1] into a C variable.

		// Serial communication
		sprintf(buf, "[%c]", triggerVal);
#ifdef DEBUGGING
		mexPrintf("%s\n", buf);
#endif
		ret = write(deviceID[0], buf, 3);
#ifdef DEBUGGING
		mexPrintf("Wrote %d byte(s).\n", ret);
#endif

		// Free memory
//		memset(buf, '\0', BUFSIZE);
//		mxFree(subCmd);
	}
	else if (strcmp(subCmd, "r") == 0)
	{
		// Argument processing
		if (nrhs < 2)
			mexErrMsgTxt("When the first parameter is 'r', the second parameter must be an integer, indicating the number of bytes to read.");
		else
			nBytesToRead = (int)*mxGetPr(prhs[1]);	// copy the numeric data from prhs[2] into a C variable.

		// Serial communication
		int ret = 0;
		ret = read(deviceID[0], buf, nBytesToRead);
#ifdef DEBUGGING
		mexPrintf("Read: %s\n", buf);
#endif
		mexPrintf("Read: %d bytes\n", ret);

		// Free memory
		mxFree(subCmd);
	}
	else if (strcmp(subCmd, "w") == 0)
	{
		// Argument processing
		if (nrhs < 2)
			mexErrMsgTxt("When the first parameter is 'w', the second parameter must be the string to send.");
		else
			strParamBuf = mxArrayToString(prhs[1]);	// copy the string data from prhs[1] into a C string strParamBuf.

		// Serial communication
		//int z = write(deviceID[0], "!", 1);
		int z = write(deviceID[0], strParamBuf, strlen(strParamBuf));
#ifdef DEBUGGING
		mexPrintf("Wrote %d byte(s).\n", z);
#endif

		// Free memory
		mxFree(subCmd);
		mxFree(strParamBuf);
	}
	else if (strcmp(subCmd, "c") == 0)
	{
		if (close(deviceID[0]) != 0)
			mexPrintf("close() does not seem to be successful.");
		else
		{
			mexPrintf("Port closed.\n");
			deviceID[0] = 0;
		}

		// Free memory
		mxFree(subCmd);
	}
	else if (strcmp(subCmd, "x") == 0)
	{
		// Resetting
		// Serial communication
		ret = write(deviceID[0], "`", 1);
		if (ret < 0)
			mexErrMsgTxt("write() error during handshake");
		else
			mexPrintf("Reset device.");

		if (close(deviceID[0]) != 0)
			mexPrintf("close() does not seem to be successful.");
		else
		{
			mexPrintf("Port closed.\n");
			deviceID[0] = 0;
		}

		// Free memory
		memset(buf, '\0', BUFSIZE);
		mxFree(subCmd);
	}
	else
		mexPrintf("The first parameter can be '(o)pen', '(d)ebug', (r)ead', '(w)rite', or '(c)'lose.\n");

	return;
}
