%% Diseño de un filtro pasabajas digital
% Parametros continuos del filtro
Fs = 1000; % Frecuencia de muestreo [Hz]
Fn = Fs/2; % Frecuencia de Nyquist [Hz]
Fc = 50;    % Frecuencia de corte Fc < Fn [Hz]

% Parámetros del filtro digital
wc = 2*pi*Fc/Fs;    % Frecuencia de corte digital [-pi < wc < pi]
M = 11;             % Numero de coeficientes (TAPS) del filtro, impar

% Calculo de los coeficientes del filtro
h = zeros(1,M);

% Ejemplo para M = 5
% h(1) = wc/pi * sin(-2*wc) / (-2*wc); % h[-2]
% h(2) = wc/pi * sin(-1*wc) / (-1*wc); % h[-1]
% h(3) = wc/pi; % h[0]
% h(4) = wc/pi * sin(1*wc) / (1*wc); % h[1]
% h(5) = wc/pi * sin(2*wc) / (2*wc); % h[2]

% Calculo de los coeficientes del filtro digital
for i = 1 : M
    % h[n] = wc/pi sinc[wc*n]
    n = -(M-1)/2 + i - 1;
    h(i) = wc/pi * mysinc(wc*n);
end

% Grafica de la respuesta al impulso
stem(-(M-1)/2:(M-1)/2, h);

% Verificación del Filtro diseñado
n = 0:99;
x = sin(2*pi*100/Fs *n) + sin(2*pi*10/Fs *n);
y = conv(x,h); % y[n] = x[n] * h[n]

% Grafica 
figure;
stem(x)
hold on
stem(y)
hold off
legend("x[n]", "y[n]");