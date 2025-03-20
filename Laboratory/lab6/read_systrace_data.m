function [x,t] = read_systrace_data(filename)
	% Read the data using textscan
	fid = fopen(filename, 'r');
	data = textscan(fid, '%s %s %s %s %s', 'Delimiter', ';');
	fclose(fid);

	% Extract PCM values and timestamps
	tmp = data{2};  % Second column contains PCM values
	x = zeros(length(tmp),1);
	t = zeros(length(tmp),1);
	for n = 1:length(tmp)
		x(n)  = str2double(tmp{n}(2:end-1))/(2^15);
	end
	
	tmp = data{4};
	for n = 1:length(tmp)
		% Timestamps in ms
		t(n)  = 1000*str2double(tmp{n}(2:end-1))/168000000;
	end
end