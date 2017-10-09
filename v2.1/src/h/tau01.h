//Header file for TAU01

#define TEMP_HUM_SENS_TYPE  1   //1 si es Sensirion, 2 si es otro
#define LCD_TYPE            1   //1 si es phillips, 2 si es microchip
#define LCD_ADDR            0x4E //I2C slave address for LCD module 

#define TEMP_GND_PIN        PIN_D6

//#define SIMULATION          true
#ifndef SIMULATION
  #define SIMULATION        false
#endif

#if F_CLOCK == 10
	#define  FREQ_XTAL      10MHz
	#define  FREQ_CLOCK     10MHz
#elif F_CLOCK == 12
	#define  FREQ_XTAL      12MHz
	#define  FREQ_CLOCK     12MHz
#endif

#define  BAUD_RATE          9600
#define  I2C_BUS_SCL        PIN_B4
#define  I2C_BUS_SDA        PIN_B5
#define  I2C_BUS_STREAM     i2cPort

#define  BUTTON1            PIN_E2
#define  BUTTON2            PIN_D7

#define  TX_BUFFER_SIZE     100
#define  RX_BUFFER_SIZE     100

#define  TRIS_A             0b11111111
#define  TRIS_B             0b11110111

#define  RTC_IO             PIN_D2
#define  RTC_SCLK           PIN_D1
#define  RTC_RST            PIN_D0
#define  SIM900_PWR         PIN_E1
#define  WIND_SPEED         PIN_A4

#define  SZ_SMSTXT           160          // Cantidad de caracteres x SMS
#if ADC_RES == 10
	#define  KTEMPX          0.1086       // Constante para conversor A/D
	#define  KTEMPX2         10.66        // Constante para conversor A/D
#elif ADC_RES == 16
	#define  KTEMPX          0.0271       // Constante para conversor A/D
	#define  KTEMPX2         10.58        // Constante para conversor A/D
#endif

#define  KTE_PLUV            0.2          // Constante de pluviometro
#define  PRE_ACUM            48           // Cantidad de medidas a acumular de lluvia
#define  VOLT_DIVIDER        3.3
#define  VCC		         5.09
#define  WIND_SPEED_PREC	 0.4712       // Precision del sensor de viento
#define  WIND_SPEED_TIME     2            // Tiempo de muestreo para calcular v. viento (seg)

#if ADC_RES == 10
	#define	 MAX_ADC_RESOLUTION 1024
#elif ADC_RES == 16
	#define	 MAX_ADC_RESOLUTION 4096
#endif

#define  ADC_STEP           (VCC/MAX_ADC_RESOLUTION)   // 0.004971 --- 5.09 Vhref+ divido MAX_ADC_RESOLUTION
#define  KTE_RADIATION      290.5        // Cantidad de mV cada 1000 W/m2
#define  OFFSET_WATER_LEVEL 164          // Offset en mV. (CERO del FREATIMETRO)
#define  KTE_WATER_LEVEL    5            // Profundidad a la que se ubica el sensor en m.
#define  WATER_LEVEL_1_M    859          // En mV. Salida del sensor cada 1 m de agua.

#define  MAX_CNALARM        3            // Cantidad de mediciones seguidas para activar alarma

#define  TEMP_GND_S         0            // Bit estado sensor de temperatura de suelo.
#define  TEMP_HUM_S         1            // Bit estado sensor de temperatura y humedad.
#define  RAINFALL_S         2            // Bit estado sensor de precipitacion.
#define  PRESSURE_S         3            // Bit estado sensor de presion.
#define  HUM_GND_S          4            // Bit estado sensor humedad de suelo.
#define  RADIATION_S        5            // Bit estado sensor radiacion.
#define  WIND_S             6            // Bit estado sensor de viento.
#define  OTHER_S            7            // Bit estado sensor GENERICO.
#define  MAX_CONF_SENS		15			       // Cantidad de sensores a configurar como habilitado o deshabilitado

#if F_CLOCK == 10
	#define  T0OF_SECS      6.7109      // Para 10 Mhz. Segundos que tarda en desbordar Timer0. 
#elif F_CLOCK == 12
	#define  T0OF_SECS      5.5923      // Para 12 Mhz. Segundos que tarda en desbordar Timer0
#endif

#define  REPORT_MINUTES     30
#define  ALARM_MINUTES      5
#define  RESTART_MINUTES    51
#define  ALARM_SLEEP        12

#define  TYP_SMS_ON         0            // Encendido
#define  TYP_SMS_CON        1            // Consulta de variables actuales
#define  TYP_SMS_AA         2            // Alta de una alarma
#define  TYP_SMS_BA         3            // Baja de una alarma
#define  TYP_SMS_INI        4            // Inicializar eprom
#define  TYP_SMS_RTC        5            // Poner en fecha y hora el rtc
#define  TYP_SMS_CSERV      6            // Cambiar numero de tel a reportar
#define  TYP_SMS_CSH        7            // Cambiar la configuracion de los sensores habilitados
#define  TYP_SMS_RESTART    8            // Reinicia la estación remotamente
#define  TYP_SMS_ERR        240          // Error

#define  CSH_ADDR           0x00
#define  TEL_ADDR           0x01         // Inicio memoria Numero Telefono (20 bytes) ( NO ESTA EN USO )
#define  DEBUG_ADDR         0x02         // DEBUG Mode
#define  SZ_TELEFONO        20           // Tamaño del telefono
#define  AA_ADDR            0x03         // Inicio memoria alarmas
#define  SZ_ALRMSTR         60           // Tamaño registro de alarma

#use delay(xtal=FREQ_XTAL, clock=FREQ_CLOCK, restart_wdt)
#use rs232(baud=BAUD_RATE, uart1, stream=SIM900, parity=N, bits=8, stop=1, errors, disable_ints)
#use i2c(master, scl=I2C_BUS_SCL, sda=I2C_BUS_SDA, slow, stream=I2C_BUS_STREAM, force_sw)
#use fast_io(A)
#use fast_io(B)

#use standard_io(C)
#use standard_io(D)
#use standard_io(E)

#if LCD_TYPE == 2
  #include <i2cLCD_MCP.c>
#elif LCD_TYPE == 1
  #include <i2cLCD_PCT.c>
#endif

#if TEMP_HUM_SENS_TYPE == 1
  #include <sht75.c>
#elif TEMP_HUM_SENS_TYPE == 2
  #include <dht22.c>
#endif

#include <debug_utils.c>  
#include <modem.c>
#include <ds1302.c>
#include <timeFunc.c>
#include <ds1820.c>
//#include "h/ds1820.h"