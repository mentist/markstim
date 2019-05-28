% Version: 2018-01-09~2018-01-10
% Author: Yong-Jun Lin
%
% References:
% http://www.instructables.com/id/Arduino-and-Matlab-let-them-talk-using-serial-comm/
% https://www.mathworks.com/Matlabcentral/answers/255495-serial-communication-read-write-from-to-arduino-on-Matlab-support-package-for-arduino-hardware
% https://www.mathworks.com/Matlabcentral/answers/325725-sending-values-from-Matlab-to-arduino-using-serial-communication?requestedDomain=www.mathworks.com
%
% History:
% 2018-01-09 YJL Controlled LED by serial communication

% Copyright (C) 2013-2019  Yong-Jun Lin
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

baudRate = 57600;
system('ls /dev | grep usbmodem');
teensy = serial('/dev/cu.usbmodem12341', 'BaudRate', baudRate);
fopen(teensy)
for t = 1:10
	fprintf(teensy, '%s', '`')
	pause(0.5)
	fprintf(teensy, '%s', ' ')
	pause(0.5)
end
fclose(teensy)
