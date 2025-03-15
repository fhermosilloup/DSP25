%% Ejercicio. Grafique la CTFT de x[t] = e^{-at} u[t]
% X[W] = 1/[a + jW]

% Parametro de X[W]
a = 2;

% Eje de frecuencia
N = 1000;   % Numero de puntos + 1 a graficar
F = 50;     % Frecuencia máxima a graficar
W = -2*pi*F : 4*pi*F/N : 2*pi*F;    % Eje de frecuencia

% Señal Xw
Xw = 1 ./ (a + 1i*W);

% Grafica
plot(W, abs(Xw));
title("Magnitud");
figure;
plot(W, angle(Xw)*180/pi);
title("Fase");
