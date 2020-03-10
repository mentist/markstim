% Version: 2018-01-09~2018-01-17
% Author: Yong-Jun Lin
%
% History:
% 2018-01-09 YJL Controlled LED by serial communication.
% 2018-01-11 YJL Reduced the test case to character echoing.
%                Instead of echoing, respond by ++ to an incoming character.
%                Tested whether CR+LF will be transmitted as part of the string.
% 2018-01-15 YJL Completed the basic communcation with MarkStim('o'), MarkStim('w'), MarkStim('r'), and MarkStim('c').
% 2018-01-17 YJL Simplified the steps with MarkStim('i'), MarkStim('s'), MarkStim('t'), and MarkStim('x').
%
% References:
% http://www.instructables.com/id/Arduino-and-Matlab-let-them-talk-using-serial-comm/
% https://www.mathworks.com/Matlabcentral/answers/255495-serial-communication-read-write-from-to-arduino-on-Matlab-support-package-for-arduino-hardware
% https://www.mathworks.com/Matlabcentral/answers/325725-sending-values-from-Matlab-to-arduino-using-serial-communication?requestedDomain=www.mathworks.com

% Copyright (C) 2013-2020  Yong-Jun Lin
% This file is part of MarkStim, a TMS trigger/EEG event registration 
% device. See <https://yongjunlin.com/MarkStim/> for the documentation 
% and details.
%
% This program is free software: you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation, either version 3 of the License, or
% (at your option) any later version.
%
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
%
% You should have received a copy of the GNU General Public License
% along with this program.  If not, see <https://www.gnu.org/licenses/>.


%if ~exist('MarkStim.mexmaci64', 'file')
%	mex MarkStim.c
%end

system('ls /dev | grep usbmodem12341');

MarkStim('i', '/dev/cu.usbmodem12341')	% Open connection and handshake
MarkStim('s', 1, 5000)	% bTTL=true; TTLPulseWidth=1000 microsec

for e = 0:7
	pause(0.25)
	MarkStim('t', 2^e)
end

MarkStim('t', 255)

% N = 100;
% dur = nan(1, N);
% for i = 1:N
% 	tic
% 	MarkStim('w', ['[' char(30)])
% 	MarkStim('r', 1)
% 	dur(i) = toc;
% end
% 
% MarkStim('c')
% 
% plot(1000*dur)
% ylabel('(ms)')
% mean(dur)

MarkStim('x')	% Close serial connection and reset device
