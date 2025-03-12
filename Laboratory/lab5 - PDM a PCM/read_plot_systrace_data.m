% Load the CSV file (assuming no headers)
filename = "systrace_data.csv";

% Read the data using textscan
fid = fopen(filename, 'r');
data = textscan(fid, '%s %f %s %f %s', 'Delimiter', ';');
fclose(fid);

% Extract PCM values and timestamps
pcm_values = data{2};  % Second column contains PCM values
timestamps = 1000*data{4}/168000000;  % Fourth column contains timestamps in ms

% Plot PCM waveform
figure;
plot(timestamps, pcm_values, 'b');
xlabel('Time [ms]');
ylabel('Amplitude [Raw]');
title('PCM Data Captured from SysTrace');
grid on;
