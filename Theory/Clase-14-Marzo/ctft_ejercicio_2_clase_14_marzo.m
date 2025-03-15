%% Ejercicio. Grafique la CTFT de x[t] = sin[W0*t]
% Parametro de X[W]
F0 = 100;
W0 = 2*pi*F0;

% Eje de frecuencia
N = 1000;   % Numero de puntos + 1 a graficar
F = 2*F0;   % Frecuencia máxima a graficar
W = -2*pi*F : 4*pi*F/N : 2*pi*F;    % Eje de frecuencia

% Señal X[W] = j*pi * [d[W + W0] - d[W - W0]]
Xw = zeros(length(W));

% En W = W0 -> -i*pi -> -i*pi*d[W - W0]
Xw(W == W0) = -1i*pi;

% En W = -W0 -> i*pi -> i*pi*d[W + W0]
Xw(W == -W0) = 1i*pi;

% Grafica
figure;
plot(W, abs(Xw));
title("Magnitud");
figure;
plot(W, angle(Xw)*180/pi);
title("Fase");