%%
%%
clc,clear
T=[0.0 0.4 0.5 0.6 0.8 1.0 1.2 1.3 2.2];
i=[160 170 180 190 200 210 220 225 244];

p1=polyfit(T,i,1);    %aproximaciòn por polinomio de segundo orden
p2=polyfit(T,i,2);    %aproximaciòn por polinomio de tercer orden

y2=polyval(p1,T);   %evalua el polinomio de segundo orden en los puntos x2
y3=polyval(p2,T);

plot(i,T,'o',i,y2,'r',i,y3,'g');
grid on;