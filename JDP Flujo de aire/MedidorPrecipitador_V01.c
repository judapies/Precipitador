//  faiverht@gmail.com
//////////////////////////////////////////////////
// Lector de datos análogos desde el PIC (Flujo de aire y temperatura)
// Muestra velocidad en m/s y grados Ok
// Precipitador Electrostático

// Recepción de datos USB paquetes de 10 bytes [1 byte entero y 6 bytes fracciones decimales, 1 byte índice y 1 byte de Chk_sum]
// El índice corresponde al tipo de dato enviado: [a(1), b(2), c(3), d(4), LimVel(5), LimTemp(6)]
// Después de recibidos los 6 paquetes se envía un paquete de acuse de recibo completo o incompleto
// solicitando reenvío de datos
// Modificació realizada en Cenipalma el 19 de Julio. ts=150ms, promedio con 16 muestras con cálculo e impresión cada 16

#include <18F4550.h>
#DEVICE ADC=10
//#include <MATH.H>
//#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL3,CPUDIV1,VREGEN// PLL3 para 12 MHz
#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL1,CPUDIV1,VREGEN// PLL5 para 20 MHz
//#use delay(clock=4000000)
#use delay(clock=48000000)
#include <lcd.h>
#define alarma_on output_bit( PIN_D1, 1);    // Alarma sonora
#define alarma_off output_bit( PIN_D1, 0);
#define piloto_on output_bit( PIN_D0, 1);    // Piloto azul
#define piloto_off output_bit( PIN_D0, 0);

float Velocidad=0.0,Voltaje=0.0,Factor=4.782,Prom=0.0;
int l=0,h=0;
int16 valor=0;
float promedio[8]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

// Funcion para conversor analogo-digital
int16 sensores(int x){
int16 y;set_adc_channel(x);delay_ms(100);y=read_adc();return (y);
}

void main(void) {
  lcd_init();
  lcd_enviar(lcd_comando,lcd_clear);
  lcd_gotoxy(5,1);
  printf(lcd_putc,"PRECIPITADOR");
  lcd_gotoxy(4,2);
  printf(lcd_putc,"ELECTROSTATICO");
  lcd_gotoxy(5,4);
  printf(lcd_putc,"JP INGLOBAL");
  //printf(lcd_putc,"JP INGLOBAL REF: JPCSB128");
  delay_ms(3000);
  
  delay_ms(2000);
  delay_ms(2000);
  
  
  lcd_enviar(lcd_comando,lcd_clear);
  
  lcd_gotoxy(1,1);
  printf(lcd_putc,"  VELOCIDAD: (m/s)  ");
  lcd_gotoxy(1,2);
  printf(lcd_putc,"        0.00        ");
  lcd_gotoxy(1,3);
  printf(lcd_putc,"TENSION PRECIPITADOR");
  lcd_gotoxy(1,4);
  printf(lcd_putc,"        0.0  kV     ");

//enable_interrupts(global);
//setup_adc(ADC_CLOCK_INTERNAL );
setup_adc(ADC_CLOCK_DIV_64);

setup_adc_ports( ALL_ANALOG );
  
  while (TRUE){
    for(;;){
      valor=sensores(0);
      Velocidad=(valor-162.8559)/41.6621;
      if(Velocidad<=0.0)
      {Velocidad=0.0;}
      
         if(l>=8)
            {l=0;}
         
         promedio[l]=Velocidad;l++;
         
            for(h=0;h<=7;h++)
            {
               Prom+=promedio[h];
            } 
               Prom=Prom/8;   
               
      Voltaje=(5.0*sensores(1)/1023)*Factor;


      lcd_gotoxy(1,1);
      printf(lcd_putc,"  VELOCIDAD: (m/s)  ");
      lcd_gotoxy(8,2);
      printf(lcd_putc," %1.1f   ",Prom);  // Muestra promedio de velocidad de flujo.
      lcd_gotoxy(1,3);
      printf(lcd_putc,"TENSION PRECIPITADOR");
      lcd_gotoxy(8,4);
      printf(lcd_putc," %1.1f kV   ",Voltaje);  // Muestra Promedio de valor de alta tensión.
      //lcd_gotoxy(1,2);
      //printf(lcd_putc," %1.1f   ",Voltaje/Factor);  // Muestra Promedio de valor de alta tensión.
      delay_ms(1000);
    }
    
   }
}

