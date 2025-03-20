function plot_spectrum(x,t,Fs)
	% Transformada de Fourier Discreta
	Xk = fftshift(fft(x));
	
	% Vector de frecuencia
	N=length(Xk);
	dF=Fs/N;
	if mod(N,2)==0
		n=-N/2:(N/2-1);
	else
		n=-floor(N/2):floor(N/2);
	end
	F=n*dF;
	
	% Graaficaa
	figure;
	subplot(2, 1, 1);
	plot(x);
	xlabel("Time [ms]");
	ylabel("Amplitude [V]");
	subplot(2, 1, 2);
	plot(F,abs(Xk)/N);
	xlabel("Frequency [Hz]");
	ylabel("Magnitude");
end