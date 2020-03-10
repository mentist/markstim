% Version: 2018-01-09~2018-01-11
% Author: Yong-Jun Lin
%
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


baudRate = 57600;
system('ls /dev | grep usbmodem12341');
teensy = serial('/dev/cu.usbmodem12341', 'BaudRate', baudRate);
fopen(teensy)
pause(1)
tic
fprintf(teensy, '%s', '!')
cc = char(fread(teensy, 14))';
if ~strcmp(cc, sprintf('Teensy ready\r\n'))	% https://www.arduino.cc/en/Serial/Println
	error('Handshaking error.')
end
t1 = toc;
fprintf('Handshaking took %.3f secs.\n', t1);

N = 100;
dur = nan(1, N);
for i = 1:N
	tic
	fprintf(teensy, '%s', '[')
	c = fscanf(teensy, '%s', 1);
	dur(i) = toc;
end
fclose(teensy)
plot(dur)
mean(dur)
