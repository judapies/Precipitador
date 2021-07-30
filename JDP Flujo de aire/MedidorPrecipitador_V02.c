//  faiverht@gmail.com
//////////////////////////////////////////////////
// Lector de datos análogos desde el PIC (Flujo de aire y temperatura)
// Muestra velocidad en m/s y grados Ok
// Precipitador Electrostático

#include <18F4550.h>
#DEVICE ADC=10
//#include <MATH.H>

#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL1,CPUDIV1,VREGEN// PLL5 para 20 MHz
#use delay(clock=48000000)
#include <lcd.h>
#define alarma_on output_bit( PIN_D1, 1);    // Alarma sonora
#define alarma_off output_bit( PIN_D1, 0);
#define piloto_on output_bit( PIN_D0, 1);    // Piloto azul
#define piloto_off output_bit( PIN_D0, 0);

#define USB_HID_DEVICE FALSE // deshabilitamos el uso de las directivas HID
#define USB_EP1_TX_ENABLE USB_ENABLE_BULK // turn on EP1(EndPoint1) for IN bulk/interrupt transfers
#define USB_EP1_RX_ENABLE USB_ENABLE_BULK // turn on EP1(EndPoint1) for OUT bulk/interrupt transfers
#define USB_EP1_TX_SIZE 32 // size to allocate for the tx endpoint 1 buffer
#define USB_EP1_RX_SIZE 32 // size to allocate for the rx endpoint 1 buffer

#include <pic18_usb.h> // Microchip PIC18Fxx5x Hardware layer for CCS's PIC USB driver
#include "header.h" // Configuración del USB y los descriptores para este dispositivo
#include <usb.c> // handles usb setup tokens and get descriptor reports

const int8 Lenbuf =10;  // el antepenúltimo byte (7) corresponde al índice (tipo de dato)
                        // índices: 1(dato_A), 2(dato_B), 3(dato_C), 4(dato_D), 5(Nivel_al), 6(Nivel_temp)
int8 recbuf[Lenbuf];
signed int  tempEEprom[7];
unsigned int8 sndbuf1[Lenbuf],sndbuf2[Lenbuf];
unsigned int8 i,j,indice, vidrio_Ok=1, datosLeidos=0;
unsigned int16 valor, vector[16], valorA, valorB;
float a, b, c, d, velocidad=0, tmp1, tmp2, RT, limite_inf=0.5, limite_sup=0.6, Rx=2000.0, temperatura=0.0;
float dx, dy, m, b2;

// Los datos de tabla x,y se utilizan para hacer la conversión de resistencia a temperatura
// Hace falta verificar esta relación, ya que estos datos son de la cabina anterior.
float tabla_x[11]={234.4, 310.0, 417.6, 568.9, 794.0, 1114.0, 1613.0, 1940.0, 2353.0, 3553.0, 5497.0};
float tabla_y[11]={90.0,80.0,70.0,60.0,50.0,40.0,30.0,25.0,20.0,10.0,0.0};

// Función que calcula el valor promedio del vector tamaño 8 o 16
unsigned int16 promedio(void){
   unsigned int16 temporal=0;
   for(j=0;j<16;j++){
      temporal=temporal+vector[j];
   }
   temporal=temporal>>4;
   return temporal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// M A I N
//
///////////////////////////////////////////////////////////////////////////////////////////////////
void main(void) {
   // Inicializar el vector de datos con ceros
   for(i=0;i<16;i++){
      vector[i]=0;
   }
   alarma_off;
   piloto_off
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
  
  //sndbuf1[Lenbuf]=(51,12,0,10,1,5,127,30,0,102);
  lcd_gotoxy(5,1);
  //printf(lcd_putc,"USB off ...     ");
  delay_ms(2000);
  
  usb_init();
  
  delay_ms(2000);
  
  usb_task();
  
  delay_ms(2000);
  delay_ms(2000);
  
  lcd_enviar(lcd_comando,lcd_clear);
  
  lcd_gotoxy(3,2);
  printf(lcd_putc,"VELOCIDAD: (m/s)");
  lcd_gotoxy(7,3);
  printf(lcd_putc,"  0.00  ");
  lcd_gotoxy(2,4);
  printf(lcd_putc,"REF: JPCEGH180ATD.");

setup_adc(ADC_CLOCK_DIV_64);

setup_adc_ports( ALL_ANALOG );

set_adc_channel(0);
delay_us(10);
sndbuf1[0]=0;
sndbuf1[2]=0;
sndbuf1[3]=0;

i=0;
  
  a=0.00000453;
  b=-0.0020894;
  c=0.355;
  d=-21.114;
// Actualizar datos de memoria no volátil (coeficientes de linealización)
tempEEprom[0]=read_eeprom(11);delay_ms(25);   // unidades
tempEEprom[1]=read_eeprom(12);delay_ms(25);   // No/10^2
tempEEprom[2]=read_eeprom(13);delay_ms(25);   // No/10^4
tempEEprom[3]=read_eeprom(14);delay_ms(25);   // No/10^6
tempEEprom[4]=read_eeprom(15);delay_ms(25);   // No/10^8
tempEEprom[5]=read_eeprom(16);delay_ms(25);   // No/10^10
tempEEprom[6]=read_eeprom(17);delay_ms(25);   // No/10^12

tempEEprom[0]=read_eeprom(21);delay_ms(25);   // unidades
tempEEprom[1]=read_eeprom(22);delay_ms(25);   // No/10^2
tempEEprom[2]=read_eeprom(23);delay_ms(25);   // No/10^4
tempEEprom[3]=read_eeprom(24);delay_ms(25);   // No/10^6
tempEEprom[4]=read_eeprom(25);delay_ms(25);   // No/10^8
tempEEprom[5]=read_eeprom(26);delay_ms(25);   // No/10^10
tempEEprom[6]=read_eeprom(27);delay_ms(25);   // No/10^12

tempEEprom[0]=read_eeprom(31);delay_ms(25);   // unidades
tempEEprom[1]=read_eeprom(32);delay_ms(25);   // No/10^2
tempEEprom[2]=read_eeprom(33);delay_ms(25);   // No/10^4
tempEEprom[3]=read_eeprom(34);delay_ms(25);   // No/10^6
tempEEprom[4]=read_eeprom(35);delay_ms(25);   // No/10^8
tempEEprom[5]=read_eeprom(36);delay_ms(25);   // No/10^10
tempEEprom[6]=read_eeprom(37);delay_ms(25);   // No/10^12

tempEEprom[0]=read_eeprom(41);delay_ms(25);   // unidades
tempEEprom[1]=read_eeprom(42);delay_ms(25);   // No/10^2
tempEEprom[2]=read_eeprom(43);delay_ms(25);   // No/10^4
tempEEprom[3]=read_eeprom(44);delay_ms(25);   // No/10^6
tempEEprom[4]=read_eeprom(45);delay_ms(25);   // No/10^8
tempEEprom[5]=read_eeprom(46);delay_ms(25);   // No/10^10
tempEEprom[6]=read_eeprom(47);delay_ms(25);   // No/10^12

  // Limpiar el vector de datos
  for(i=0;i<16;i++)
   {vector[i]=0;}
   i=0;
   
  while (TRUE){
    //if(usb_enumerated())
    {
    set_adc_channel(0);
    for(;;){
      delay_ms(100);
      valor = read_adc();
      lcd_gotoxy(8,3);
      printf(lcd_putc," %Lu   ",valor);
    }
    // ************** Lectura de sensor de velocidad ***************
    set_adc_channel(0);
    delay_ms(100);
    valor = read_adc();
    //lcd_gotoxy(13,1);
    //printf(lcd_putc," %Lu   ",valor);    // Tipo long sin signo
    
    vector[i]=valor;                     // Almacena lectura actual en vector
    i++;
    if(i>15)
      {i=0;
      valor = promedio();                  // Calcula el valor medio
      tmp1=valor*valor; // valor^2
      tmp2=tmp1*valor; // valor^3
      velocidad=a*tmp2+b*tmp1+c*valor+d;
      if(velocidad<0){velocidad=0;}
      lcd_gotoxy(8,3);
      //printf(lcd_putc," %f   ",velocidad);    // Tipo long sin signo
      printf(lcd_putc," %Lu   ",valor);
      }
      
   //****************** Lectura de sensor térmico ******************
   // Cálculo de la corriente y resistencia térmica
   set_adc_channel(3);
   delay_us(50);
   valorA = read_adc();
   set_adc_channel(2);
   delay_us(50);
   valorB = read_adc();
   // La R es de 2000 Ohm
   // Cálculo de la corriente I
   // I=(valorA-valorB)/(204.8*100)    (1024/5=204.8) y R=100 Ohm
   // I=(valorA-valorB)/20480 (Amp)
   
   // Cálculo de la resistencia RT
   // RT=valorB/(204.8*I) (Ohm)
   // RT=valorB*100/(valorA-valorB)  Rx= 100 Ohm
     
   tmp1=valorB*Rx;
   tmp2=(valorA-valorB);
   RT=tmp1/tmp2;
   
   //lcd_gotoxy(5,4);
   //printf(lcd_putc," %4.1f = ",RT);
      // ************** Conversión a temperatura **************
   //tmp1=valor*valor; // valor^2
   //tmp2=tmp1*valor; // valor^3
   //velocidad=a*tmp2+b*tmp1+c*valor+d;
   for(indice=0;indice<11;indice++)
      {if(RT<tabla_x[indice])
         {break;}
      }

   // Cálculo de temperatura mediante interpolación. Puntos medios
   dx=tabla_x[indice]-tabla_x[indice-1];
   dy=tabla_y[indice]-tabla_y[indice-1];
   m=dy/dx;
   b2=tabla_y[indice]-m*tabla_x[indice];
   temperatura=m*RT+b2;
   

   delay_ms(200);
   } // Cierre de PIC enumerado

  }
}


/* Pruebas a realizar Junio 19 2014

*/
