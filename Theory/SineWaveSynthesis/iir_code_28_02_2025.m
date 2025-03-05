% Parametros del filtro
Fs = 1000;      % Frecuencia de muestreo
F=200;           % Frecuencia deseada F < Fs/2
w=2*pi*F/Fs;    % Digital angular frequency -PI < w < PI
b = sin(w);     % 
a = cos(w);     %

N=100;              % Numero de muestras a evaluar
xprev=0;            % Buffer para almacenar valores previos de x[n] -> x[n-1]
yprev=zeros(1,2);   % Buffer para almacenar valores previos de y[n] -> [y[n-1], y[n-2]
y = zeros(1,N);     % Buffer para almacenar la salida y[n] del sistema

% Bucle para calcular la ecuación en diferencia
for n = 0 : N-1
    % Muestrear la entrada
    %  En este caso x[n] = delta[n] para calcular y[n] = h[n] = sin[w*n]
    if n == 0
        xn = 1; % d[0] = 1
    else
        xn = 0; % d[n] = 0, n > 0
    end
    
    % Calcular la salida y[n]
    %  y[n] = sen[w]*x[n-1] + 2cos[w]*y[n-1] - y[n-2]
    y(n+1) = b*xprev + 2*a*yprev(1) - yprev(2);

    %  Actualizar buffer de valores previos para el siguiente tiempo
    xprev = xn;             % x[n-1] = x[n]
    yprev(2) = yprev(1);    % y[n-2] = y[n-1]
    yprev(1) = y(n+1);      % y[n-1] = y[n]
end

% Graficar la salida
stem(y);