#include <18F4610.h>
#FUSES PUT                      //Power Up Timer
#FUSES NOLVP                    //No low voltage progming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES PROTECT                  //Code protected from reads
#FUSES HS			//High speed
#FUSES WDT128
#device ICD=TRUE
#device adc=10
#define FREQ_OSC 8000000 
#define BAUD_RATE 9600
#use delay(clock=FREQ_OSC,RESTART_WDT)
//--------------------------------------------
// Configuracion del Puerto del GSM
// Nota:  modificar Pines y baudrate de acuerdo a la config de la placa
// Importante: ERRORS se utiliza para que la UART del PIC contemple variaciones en el baudrate
//--------------------------------------------
#use rs232(baud=BAUD_RATE, xmit=PIN_C6,rcv=PIN_C7,errors,stream=SIM300)   

//---------------------- IO Pines ----------------------------------------
#define KIOPTOA     0b11110011 // E/S puerto A 
#define KIOPTOB     0b11110111  // E/S puerto B
#define KIOPTOC     0b10000001  // E/S puerto C

#define RTC_IO   	PIN_D2	// Pin I/O de rtc
#define RTC_SCLK 	PIN_D1	// Pin Sclk de rtc
#define RTC_RST  	PIN_D0	// Pin reset de rtc
//
#define EEPROM_SDA  PIN_B5  // Pin SDA memoria
#define EEPROM_SCL  PIN_B4  // Pin SCL memoria
//
#define	INP_PREC	PIN_B0	// Entrada pluviometro
#define	SIM300_PWR	PIN_E1	// Encendido SIM300
//
#define	TX_BUFFER_SIZE	100
#define	RX_BUFFER_SIZE	100
//
// --------------------- Constantes --------------------------------------
//
#define SZ_SMSTXT       165				// Cantidad de caracteres x SMS 
#define KTEMPX   		0.1091          // Constante para conversor A/D
#define KTEMPX_QTY		15              // Constante que define la cantidad de medidas a tomar para calcular la presion
#define KTEMPX_CORR     0               // Constante de correccion de la medida del sensor
#define KTESHT_QTY		5               // Constante que define la cantidad de medidas a tomar para calcular la humedad y presion
#define K_TIME_MED		1740			//  Segundos de intervalo entre mediciones
#define K_TIME_CHK		1500    		// Segundos para control de mediciones internas
#define K_TIME_RESTART	6000    		// Segundos entre intervalos de reinicio del modem
#define KTE_PLUV		0.44			// Constante de pluviometro
#define K_LLUVIA_H		24				// Cantidad de horas para obtener el acumulado de lluvia
#define KTE_MAXTI		60.0			// Máxima temperatura interna del equipo
#define KTE_MINTI		0.0				// Mínima temperatura interna del equipo
#define MAX_CNALARM		3				// Cantidad de mediciones seguidas para activar alarma
#define HAB_SEN_TI		0x01			// Sensor de temperatura interna habilitado
#define TEMP_HUM_S		0x02			// Sensor de temperatura y humedad habilitado
#define RAINFALL_S		0x04			// Sensor de precipitacion habilitado
#define PREASURE_S		0x08			// Sensor de presion habilitado
#define HAB_SENS5		0x10			// Reservado
#define HAB_SENS6		0x20			// Reservado
#define HAB_SENS7		0x40			// Reservado
#define HAB_SENS8		0x80			// Reservado

// -------------------------- RESPUESTAS----------------------------------
#define TYP_SMS_ON		0			// Encendido
#define TYP_SMS_CON		1			// Consulta de variables actuales
#define TYP_SMS_AA		2			// Alta de una alarma
#define TYP_SMS_BA		3			// Baja de una alarma
#define TYP_SMS_INI		4			// Inicializar eprom
#define TYP_SMS_RTC		5			// Poner en fecha y hora el rtc
#define TYP_SMS_CSERV	6			// Cambiar numero de tel a reportar
#define TYP_SMS_ERR		240			// Error

//------------------- EEPROM INTERNA ----------------------------------
#define ADDR_HAB_L		0x00			// Inicio memoria sensores habilitados
#define ADDR_HAB_H		0x01
#define ADDR_TMED_L		0x02			// Inicio memoria tiempo de medicion programado
#define ADDR_TMED_H		0x03
#define ADDR_TCHK_L		0x04			// Inicio memoria tiempo de control interno programado
#define ADDR_TCHK_H		0x05

#define ADDR_TELEF1		0x0A			// Inicio memoria Numero Telefono (20 bytes)
#define ADDR_TELEF2		0x1E			// Inicio memoria Numero Telefono (20 bytes)
#define ADDR_ALARM		0x32			// Inicio memoria alarmas

#define SZ_TELEFONO		14				// Tamaño del telefono
	
//------------------- UMBRALES INTENSIDAD DE LLUVIA  ------------------------------
#define U_DEBIL        		1
#define U_MOD	       		7.5
#define U_FUERTE       		15
#define U_MUYFUERTE    		30

#rom 0x0F00000={(HAB_SEN_TI | TEMP_HUM_S | RAINFALL_S | PREASURE_S), K_TIME_MED, K_TIME_CHK}
#rom (0x0F00000 +ADDR_ALARM) = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

//------------------- EEPROM EXTERNA ----------------------------------
#define MAX_LOG_CNT		1000			// Maximo nro. de registros
#define CN_MEMDATA_L	0x0000			// Contador registros
#define CN_MEMDATA_H	0x0001			// Contador registros
#define INI_LOG_MEM		0x0002			// Inicio memoria LOG
#define FIN_LOG_MEM		0xD7A2			// Fin Memoria LOG
#define INI_AUXEE		0xD7A3			// Inicio auxiliar
//------------------- Includes ----------------------------------------
#include "modem.c"
#include <ds1302.c>
#include "sht75.c"
#include <24512.C>
#include "timeFunc.c"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

long LeerSmsCmd(char *sms) ;
void Leer_sensores(char prog);
void GenerateToken(void);
void clear_var(char* var);
void verifica_eeprom(void);
long EnviarSms(char *sms, char *tel);
float obtenerPresion(void);
char obtenerIntensidad(int32 seconds, float countPrec);
void calcularTempHum(float & temp, float & truehumid); 
void inicializa_eeprom(void);
float obtenerTempInt(void);
void reinicioOK(void);
void construirTextoSms(char* sms, char prog);
void concatVariableSeparator(char* sms);

// ------------------ Comienzo código -----------------------------------
#use fast_io(A)
#use fast_io(B)
#use fast_io(C)
char telnum[] = "+5493416527105";
char sepVar[] = "*";
//char telnum[] = "+5493415460866";
//------------------- Variables usadas como Banderas -------------------
short		flagInt2Edge;
long  		adc_val, countPrec, timeMed, timeMedTmp, currSmsId, timeChkTmp, timeChk, timeRestart;			//16bits
float 		ctePluv;		
int   		runOk, resProc, idLastSmsOut, memoryFull, smsMemSnd, startMeasure, startCheck, startRestart,llueveAhora;
int32  	    secUltMed;	
BYTE		sensorHab;	// Tiene el estado de los sensores habilitados
char  		tokenSrv[3], tokenEma[3], smsText[SZ_SMSTXT], smsOut[SZ_SMSTXT], telaux[SZ_TELEFONO], charFloat1[10];
long  		address, contData;
BYTE		data, temp;

struct medSrt{
	int32	dateTime;			// 4 Bytes Fecha y hora en binario
	float	humedad;			// 4 Bytes
	float	tempExt;			// 4 bytes
	float	precip;				// 4 Bytes
	char	intensidad;			// 1 Bytes
	float	preasure;			// 4 bytes
	long	reservado;			// 1 byte (reservado)
};								// 22 Bytes

struct medSrt dataMed;
struct medSrt readMed;

#define SZ_DATALOG	sizeof(dataMed)		// Tamaño de los registro de log (20)

struct alarmaSt {
	char	activa;				// Indica alarma activa
	char	variable;			// Variable a ser monitoreada
	float	max_value;			// Valor maximo que dispara alarma
	float	min_value;			// Valor minimo que dispara alarma
	int8	cnValid;			// Contador para validar alarma
};								// 1 + 1 + 4 + 4 + 1 = 11 bytes

struct alarmaSt alarm[5];			// Matriz de deficion de alarmas 55 bytes

#define SZ_ALRMSTR	55			// Tamaño registro de alarma

/*--------------------------------------------------------------------------------------------------------------
|  void ReadAlarmStruct(void)	Lee alarmas guardadas en eeprom interna
|
|
--------------------------------------------------------------------------------------------------------------*/
void ReadAlarmStruct(void)
{
	long	addtmp;
	char	*p;
	int		i;
	
	p = &alarm[0];
	addtmp = ADDR_ALARM;
	for (i = 0; i < SZ_ALRMSTR; i++){
		//TODO
		//*p = read_eeprom(addtmp);
		p++;
		addtmp++;
	}
}
//--------------------------------------------------------------------------------------------------------------


/*--------------------------------------------------------------------------------------------------------------
|  void WriteAlarmStruct(void)	Escribe alarmas en eeprom interna
|
|
--------------------------------------------------------------------------------------------------------------*/
void WriteAlarmStruct(void)
{
	long	addtmp;
	char	*p;
	int		i;
	
	p = &alarm[0];
	addtmp = ADDR_ALARM;
	for (i = 0; i < SZ_ALRMSTR; i++){
		//TODO write_eeprom (addtmp, *p);
		p++;
		addtmp++;
	}
}
//--------------------------------------------------------------------------------------------------------------

/*--------------------------------------------------------------------------------------------------------------
| int CheckAlarmValues(void)  
|
|
--------------------------------------------------------------------------------------------------------------*/
int CheckAlarmValues(void)
{
	int i, res;
	char auxSms[10];
	Leer_sensores(1);						// Lee sensores
	res = 0;
	for (i = 0; i < 5; i++){
		if (alarm[i].activa){
			switch(alarm[i].variable){
				case 'T':					// Temperatura
					if (dataMed.tempExt > alarm[i].max_value){		// Verifica condicion de maximo
						alarm[i].cnValid++;
						if (alarm[i].cnValid > MAX_CNALARM){ 
							res = 0x01;							// Activa condicion de alarma por maximo
							alarm[i].cnValid = 0;					// Pone contador de condicion de alarma en 0
							GenerateToken();
							sprintf(auxSms, "%4f", dataMed.tempExt);
							auxSms[4] = 0;
							sprintf(smsOut, "A|T|%s|%c%c", auxSms, tokenEma[0], tokenEma[1]);
							idLastSmsOut = EnviarSms(smsOut, telnum);	
							return res;
						}
					}
					else{ 
						if (dataMed.tempExt < alarm[i].min_value){	// Verifica condicion de minimo
							alarm[i].cnValid--;
							if ((alarm[i].cnValid * (-1)) > MAX_CNALARM){ 
								res = 0x02;						// Activa condicion de alarma por mínimo
								alarm[i].cnValid = 0;				// Pone contador de condicion de alarma en 0
								GenerateToken();
								sprintf(auxSms, "%4f", dataMed.tempExt);
								auxSms[4] = 0;
								sprintf(smsOut, "A|T|%s|%c%c", auxSms, tokenEma[0], tokenEma[1]);
								idLastSmsOut = EnviarSms(smsOut, telnum);	
								return res;
							}
						}
						else{
							alarm[i].cnValid = 0;					// En estado normal pone contador de alarma en 0
						}
					}
					break;
				case 'H':					// Humedad
					if (dataMed.humedad > alarm[i].max_value){		// Verifica condicion de maximo
						alarm[i].cnValid++;
						if (alarm[i].cnValid > MAX_CNALARM){ 
							res = 0x04;							// Activa condicion de alarma por maximo
							alarm[i].cnValid = 0;
							GenerateToken();
							sprintf(auxSms, "%4f", dataMed.humedad);
							auxSms[4] = 0;
							sprintf(smsOut, "A|H|%s|%c%c", auxSms, tokenEma[0], tokenEma[1]);
							idLastSmsOut = EnviarSms(smsOut, telnum);	
							return res;
						}
					}
					else{ 
						if (dataMed.humedad < alarm[i].min_value){	// Verifica condicion de minimo
							alarm[i].cnValid--;
							if ((alarm[i].cnValid * (-1)) > MAX_CNALARM){ 
								res = 0x10;						// Activa condicion de alarma por mínimo
								alarm[i].cnValid = 0;
								GenerateToken();
								sprintf(auxSms, "%4f", dataMed.humedad);
								auxSms[4] = 0;
								sprintf(smsOut, "A|H|%s|%c%c", auxSms, tokenEma[0], tokenEma[1]);
								idLastSmsOut = EnviarSms(smsOut, telnum);	
								return res;
							}
						}
						else{
							alarm[i].cnValid = 0;					// En estado normal pone contador de alarma en 0
						}
					}
					break;
				case 'L':					// Precipitacion
				case 'P':					// Presion
				default:
					return 0;
			}
		}
	}
	return 0;
}
//--------------------------------------------------------------------------------------------------------------

/*--------------------------------------------------------------------------------------------------------------
| char reinicioOK(void);
|
|
--------------------------------------------------------------------------------------------------------------*/
void reinicioOK(void){
	int cause = restart_cause();
	delay_ms(16000);
	//output_bit(LED1, 0);
	//output_bit(LED2, 1);
	GenerateToken();
	CurrDateTime();
	sprintf(smsOut, "EST|%s|%s|ON|%i|%c%c",dtFecha, dtHora, cause, tokenEma[0], tokenEma[1]);
	EnviarSms(smsOut, telnum);
}


//--------------------------------------------------------------------------------------------------------------

/*--------------------------------------------------------------------------------------------------------------
| void concatVariableSeparator(char* sms)
| Concatena el separador de variables * , si el texto no esta vacio
|
--------------------------------------------------------------------------------------------------------------*/
void concatVariableSeparator(char* sms)
{
	if(strlen(sms) != 0) {
	    strcat(sms,sepVar);
	}
}
/*--------------------------------------------------------------------------------------------------------------
| void construirTextoSms(char* sms, char prog)
| Construye el SMS a enviar dependiendo los sensores que estan habilitados o no
|
--------------------------------------------------------------------------------------------------------------*/
void construirTextoSms(char* sms, char prog)
{
	char auxVar[SZ_SMSTXT];
	clear_var(auxVar);

	if (sensorHab & TEMP_HUM_S){		// Sensor de temperatura y humedad habilitado ?
		//Temp
		sprintf(charFloat1, "T:%2.1f",dataMed.tempExt);
		charFloat1[6] = 0;
		strcat(auxVar,charFloat1);
		//Humedad
		concatVariableSeparator(auxVar);
	 	sprintf(charFloat1, "H:%2.1f",dataMed.humedad);
		charFloat1[6] = 0;
		strcat(auxVar,charFloat1);
	}
	if (sensorHab & RAINFALL_S){		// Sensor de precipitacion habilitado ?
		//Precipitacion
		concatVariableSeparator(auxVar);
		sprintf(charFloat1, "L:%2.1f", dataMed.precip);
		charFloat1[6] = 0;
		strcat(auxVar,charFloat1);
		if (llueveAhora) {	      //Si llueveAhora = 1, mandar intensidad	
			//Intensidad
			concatVariableSeparator(auxVar);
			sprintf(charFloat1, "I:%c", dataMed.intensidad);
			charFloat1[3] = 0;
			strcat(auxVar,charFloat1);
		}
	}
	if (sensorHab & PREASURE_S){		// Sensor de presion habilitado ?
		//Presion
		concatVariableSeparator(auxVar);
		sprintf(charFloat1, "P:%4.1f", dataMed.preasure);
		charFloat1[8] = 0;
		strcat(auxVar,charFloat1);
	}
	
	clear_var(sms);
	if (prog){						// Si es programada es <> 0 
		GenerateToken();
		sprintf(sms, "D|%s|%s|%s|%c%c",dtFecha, dtHora, auxVar, tokenEma[0], tokenEma[1]);	
	}else{
		sprintf(sms, "R|%s|%s|%s|%c%c",dtFecha, dtHora, auxVar, tokenSrv[0], tokenSrv[1]);
	}
}

/*--------------------------------------------------------------------------------------------------------------
| void calcularTempHum(float & temp, float & truehumid); 
| Calcula la temperatura y humedad haciendo promedios KTESHT_QTY
|
--------------------------------------------------------------------------------------------------------------*/
void calcularTempHum(float & temp, float & truehumid)
{
	float acumTemp = 0;
	float acumHum = 0;
	char i;
	for (i = 0; i < KTESHT_QTY; i++){
		float auxTemp;
		float auxHum;
		sht_rd(auxTemp, auxHum);
		acumTemp = acumTemp + auxTemp;
		acumHum = acumHum + auxHum;
	}
	temp = acumTemp/KTESHT_QTY;
	truehumid = acumHum/KTESHT_QTY;
}
/*--------------------------------------------------------------------------------------------------------------
| float obtenerPresion(void)
|
|
--------------------------------------------------------------------------------------------------------------*/
float obtenerPresion(void)
{
	float acum = 0;
	float pre;
	set_adc_channel(0);
	delay_ms(20);
	char i;
	for (i = 0; i < KTEMPX_QTY; i++){
		adc_val = read_adc();
		while(!adc_done());
		acum = acum + (float) ((adc_val * KTEMPX) + 10.56)*10;		//*10 kPa a Hpa	// Lee presion del MPX
	}
	pre = ((acum/KTEMPX_QTY) + KTEMPX_CORR);
	return pre;
}

/*--------------------------------------------------------------------------------------------------------------
| char obtenerIntensidad(int32 seconds, float countPrec)
| Obtiene la intensidad de la lluvia teniendo en cuenta los umbrales definidos de debil, moderado, fuerte, 
| muy fuerte y torrencial
--------------------------------------------------------------------------------------------------------------*/
char obtenerIntensidad(int32 seconds, float countPrec)
{
	if ( countPrec == 0 ) {
		return 'X';	
	} else {	
		float ratio = seconds/K_TIME_MED;
		if ( countPrec <= (U_DEBIL*ratio) ) {
			return '0';
		} else  {
	        	if ( countPrec <= (U_MOD*ratio) ) {
				return '1';
			} else {
				if ( countPrec <= (U_FUERTE*ratio) ) {
					return '2';
				} else {
					if ( countPrec <= (U_MUYFUERTE*ratio) ) {
						return '3';
					} else {
						return '4';
					}
				}
			}
    		}
	}
}

//--------------------------------------------------------------------------------------------------------------
//--------------------------------------- Rutinas que atienden las interrupciones ------------------------------
#int_TIMER0
void  TIMER0_isr(void) 
{
	timeMedTmp--;
	if (timeMedTmp == 0){
		timeMedTmp = timeMed;
		startMeasure = 1;			// Indica iniciar lectura de sensores
	} 	
	timeChkTmp--;
	if (timeChkTmp == 0){
		timeChkTmp = timeChk;
		startCheck = 1;				// Indica iniciar lectura de variables de estado
	}
	timeRestart--;
	if (timeRestart == 0){
		timeRestart = K_TIME_RESTART;
		startRestart = 1;				// Indica iniciar lectura de variables de estado
	}
}

#int_EXT2
void  EXT2_isr(void){				// Rutina de atencion a la interrupcion del pluviometro
	if (flagInt2Edge == 0){   		// Flanco de Subida
  		ext_int_edge(2, H_TO_L);  	// Configuro flanco de Bajada
  		flagInt2Edge = 1;    		// Indico que el siguiente flanco será de Bajada
		countPrec++;
	    //output_bit(LED1, 1);
 	} 
	else {           				// Flanco de Bajada
   		ext_int_edge(2, L_TO_H);  	// Configuro flanco de subida
  		flagInt2Edge = 0;    		// Indico que el siguiente flanco será de Subida
	    //output_bit(LED1, 0);
 	}
}
//---------------------------------------------------------------------------------------------

//Recepción de datos de modem gsm
#int_RDA
void RDA_isr(void)
{
	  if(kbhit(SIM300)){
	    if (buf_index < BUFFER_SIZE){ 
		    buffer[buf_index]=fgetc(SIM300);
		    buf_index++; 
		    buffer[buf_index] = 0; 
	    } 
	}
}
//--------------------------------------- Rutinas que atienden las interrupciones ------------------------------

/************************************************************************************************
| void GetEEPROMMed(long inx) Obtiene medicion almacenada y llena la estructura destino
| Retorna: llena variable readMed con los valores de medicion
|
************************************************************************************************/
void GetEEPROMMed(long inx)
{
	char	*ptData, i;
	address = INI_LOG_MEM + (inx * SZ_DATALOG);
	ptData = (char*)(&readMed);
	for (i = 0; i < SZ_DATALOG; i++){
		*ptData = read_ext_eeprom(address);
		ptData++;
		address++;
	}
}   
//-------------------------------------------------------------    


/*--------------------------------------------------------------------------------------------------
GetLastRainMed() | Toma las mediciones de las ultimas XX horas de precipitaciones 
					y las retorna para su envio
---------------------------------------------------------------------------------------------------*/
long GetLastRainMed(void)
{
	long cnRain;
	signed long int i;
	int32  	dtLast, dtNew;	
	//Si no hay medidas previas retornar 0
	if (contData == 0 ) {
	  return 0;
	}
	GetEEPROMMed(contData-1);		//Ultima medicion 
	dtLast = readMed.dateTime;
	cnRain = readMed.precip;
	dtNew = dtLast - (int32)(K_LLUVIA_H * 3600);
	if (contData == 1){
		return cnRain;
	}
	for (i = (contData-2); i >= 0; i--){
		GetEEPROMMed(i);
		if (readMed.dateTime > dtNew){
			cnRain += readMed.precip;
		}
		else break;					// Acumula hasta K_LLUVIA_H horas
	}
	return cnRain;
}

/*------------------------------------------------------------------------------------------------
| Leer_sensores(char prog)
| Lee sensores y los almacena en una estructura generica
| Parametro de entrada: prog <> 0 medida programada, 0 a pedido del usuario
------------------------------------------------------------------------------------------------*/
void Leer_sensores(char prog)
{
	dataMed.dateTime = CurrDateTime();	// Obtiene la fecha y hora actual y la convierte en segundos
	if (sensorHab & TEMP_HUM_S){		// Sensor de temperatura y humedad habilitado ?
		sht_rd(dataMed.tempExt, dataMed.humedad);
		//Chequeo posible error de medida
		if (dataMed.humedad < 0 ){
			dataMed.humedad = 0;		
		}
	}
	if (sensorHab & RAINFALL_S){		// Sensor de precipitacion habilitado ?
		llueveAhora = 0;		
		if (countPrec != 0) {
			llueveAhora = 1;		
		}
		int32 seconds = (dataMed.dateTime - secUltMed);
		float actual = ((float)(countPrec * ctePluv));
		if (prog){						// Si es programada es <> 0 
			dataMed.precip = actual ;
			dataMed.intensidad = obtenerIntensidad(seconds, dataMed.precip);
			countPrec = 0;
		}
		else{
		    long acumulado = 0;
			//Quiero mostrarle al usuario, lo acumulado + los precipitado en el lapso de tiempo de la ultima  
			//medida programada y ahora
			dataMed.intensidad = obtenerIntensidad(seconds, actual);
			acumulado = GetLastRainMed()  + countPrec;
			dataMed.precip =  ((float)(acumulado * ctePluv));
		}
	}
	if (sensorHab & PREASURE_S){		// Sensor de presion habilitado ?
		dataMed.preasure = obtenerPresion();
	}
	secUltMed = dataMed.dateTime;
}

/************************************************************************************************
| StoreMed2EEPROM(void) Almacena medicion en memoria
| CN_MEMDATA_L
| CN_MEMDATA_H
| INI_LOG_MEM	
|
************************************************************************************************/
void StoreMed2EEPROM(void)
{
	BYTE	*ptData, i;

	address = CN_MEMDATA_L;
	contData = read_ext_eeprom(address);
	address = CN_MEMDATA_H;
	contData += (read_ext_eeprom(address) * 256);
	address = INI_LOG_MEM + (contData * SZ_DATALOG);
	ptData = &dataMed;
	for (i = 0; i < SZ_DATALOG; i++){
		data = *ptData;
		write_ext_eeprom(address, data);
		address++;
		ptData++;
	}
	contData++;
	address = CN_MEMDATA_L;
	data = contData & 0x00FF;
	write_ext_eeprom(address, data);
	address = CN_MEMDATA_H;
	data = contData >> 8;
	write_ext_eeprom(address, data);
	
	if (contData > MAX_LOG_CNT){
		memoryFull = 1; 
	}
}

/************************************************************************************************
| Funcion para lectura de los sms
| Parametro de entrada: variable para almacenar el contenido del sms
************************************************************************************************/
long LeerSmsCmd(char *sms) 
{
	int  rta = 0;
	char *pt, sepCh[2];	
	char *ptr;
	char *ptr2;
	int	 i;
	//Chequeo en las otras posiciones en caso que se hay encolado algun SMS
	clear_buffer_gsm();
	fprintf(SIM300,"AT+CMGR=%d,0\r",guia);
	delay_ms(2000);
	if(gprs_response(sms_noleido,1000)!=0){   // Verifica si hay un Mensaje no leido
		pt = gprs_response(telnum,1000);
		if(pt!=0){
			clear_var(smsText);
			strcpy(sepCh, "\"");
			ptr = strtok(pt, sepCh);
			do {
				ptr2 = ptr;
			} while((ptr = strtok( NULL, sepCh)) != NULL );
			strcpy(sepCh, "\r");
			strtok(NULL, sepCh);
			ptr2++;
			ptr2++;
			for (i = 0; i < SZ_SMSTXT; i++){
				if ((ptr2[i] == '\r') || (ptr2[i] == '\n')){
					ptr2[i] = 0;
					break;
				}
			}
			strcpy(sms,ptr2);
			rta = guia;
		}
		int resp = borrar_sms();
		if (!resp){
			guia++;
		}
	} 
    return rta;
}

/************************************************************************************************
| Funcion para enviar los sms
| Parametro de entrada: variable para almacenar el contenido del sms, numero donde enviar el sms
************************************************************************************************/
long EnviarSms(char *sms, char *tel)
{	
	char	tmOut;
	clear_buffer_gsm();
	fprintf(SIM300,"AT+CMGS=\"%s\"\r",tel); // Teléfono al cúal le enviaremos el mensaje.
	tmOut = 150;
	do {
		tmOut--;
	}while((gprs_response(caracter,2000) == NULL) && (tmOut != 0));
	
	fprintf(SIM300,"%s",sms);             // Imprimimos mensaje a enviar. 
	delay_ms(2000);  
	fputc(0x1A,SIM300);                   // Comando para enviar el mensaje. Equivale al CRTL+Z.
	clear_buffer_gsm();
  return 0;
}

/************************************************************************************************
| Limpia el buffer usado para guardar comunicacion con modem gsm
************************************************************************************************/
void clear_var(char* sms)  /*borra el buffer sms */
{ 
  int erase = 0; 
  while(erase < SZ_SMSTXT) {
	sms[erase] = '\0';
	erase++;
  }
} 

/************************************************************************************************
| Inicializa la EEPROM
************************************************************************************************/
void inicializa_eeprom(void)
{
	contData = 0;
	memoryFull = 0; 
	address = CN_MEMDATA_L;
	write_ext_eeprom(address, 0);
	address = CN_MEMDATA_H;
	write_ext_eeprom(address, 0);
}

/************************************************************************************************
| Verifica si la memoria EEPROM esta llena o no
************************************************************************************************/
void verifica_eeprom(void)  /*borra el buffer sms */
{ 
	//--- Verifica condicion de memoria al inicio
	address = CN_MEMDATA_L;
	contData = read_ext_eeprom(address);
	address = CN_MEMDATA_H;
	contData += (read_ext_eeprom(address) * 256);
	if (contData > MAX_LOG_CNT){
		memoryFull = 1;    
	}
	//--- Verifica condicion de memoria al inicio
} 


/************************************************************************************************
| int  ProcessMessage(char *msg)
| Analiza el string del mensaje recibido y procesa el comando correspondiente
|
************************************************************************************************/
int  ProcessMessage(char *msg)
{
	char 	aux[30], aux2[8], sepCh[2], *ptr, *telnumaux;
	BYTE  	hora, min, dia, mes, year;
	BYTE	typeAlrm, i, done;
	float	maxAlrm, minAlrm;
	
	memset(aux, 0, sizeof(aux));
	strcpy(aux, msg);
	strcpy(sepCh, "|");
	ptr = strtok(aux, sepCh);
	strcpy(aux2, "AA");
	if (strcmp(ptr, aux2) == 0) {
		//------------------------------------ Procesa Alta de alarma --------------------------------
		ptr = strtok(null, sepCh);
		if (ptr != NULL) typeAlrm = *ptr;		// Tipo alarma
		else return TYP_SMS_ERR;
		//
		ptr = strtok(null, sepCh);
		if (ptr != NULL) maxAlrm = atof(ptr);	// Valor maximo
		else return TYP_SMS_ERR;
		//
		ptr = strtok(null, sepCh);
		if (ptr != NULL) minAlrm = atof(ptr);	// Valor minimo
		else return TYP_SMS_ERR;
		//
		ptr = strtok(null, sepCh);
		if (ptr != NULL) strcpy(tokenSrv, ptr);	// Valor de token enviado por servidor
		else return TYP_SMS_ERR;
		//
		done = 0;
		for (i = 0; i < 5; i++){
			if (alarm[i].variable == typeAlrm){
				alarm[i].activa 	= 1;
				alarm[i].variable 	= typeAlrm;
				alarm[i].max_value 	= maxAlrm;
				alarm[i].min_value 	= minAlrm;
				alarm[i].cnValid	= 0;
				done = 1;
				break;
			}
		}
		if (done == 0){								// No existe variable en tabla de alarmas 
			for (i = 0; i < 5; i++){
				if (alarm[i].activa == 0){			// Actualiza valores de alarma y la activa
					alarm[i].activa 	= 1;
					alarm[i].variable 	= typeAlrm;
					alarm[i].max_value 	= maxAlrm;
					alarm[i].min_value 	= minAlrm;
					alarm[i].cnValid	= 0;
					done = 1;
					break;
				}
			}
		}
		else {
			return TYP_SMS_AA;	
		}
		if (done == 0) return TYP_SMS_ERR;		// Si no encuentra lugar libre para alarma sale con error
		//--------------------------------------------------------------------------------------------
		return TYP_SMS_AA;
	}
	else{
		strcpy(aux2, "BA");
		if (strcmp(ptr, aux2) == 0) {
			//-------------------------------- Procesa Baja de alarma --------------------------------
			ptr = strtok(null, sepCh);
			if (ptr != NULL) typeAlrm = *ptr;		// Tipo alarma
			else return TYP_SMS_ERR;
			//
			ptr = strtok(null, sepCh);
			if (ptr != NULL) strcpy(tokenSrv, ptr);	// Valor de token enviado por servidor
			else return TYP_SMS_ERR;
			//
			done = 0;
			for (i = 0; i < 5; i++){
				if (alarm[i].variable == typeAlrm){		// Busca alarma para eliminar
					alarm[i].activa 	= 0;
					alarm[i].cnValid	= 0;
					done = 1;
					break;
				}
			}
			if (done == 0) return TYP_SMS_ERR;		// Si no encuentra alarma sale con error
			//----------------------------------------------------------------------------------------
			return TYP_SMS_BA;
		}
		else{
			strcpy(aux2, "CLIMA");
			if (strcmp(ptr, aux2) == 0) {
				//---------------------------- Procesa Pedido de medicion ----------------------------
				ptr = strtok(null, sepCh);
				if (ptr != NULL) strcpy(tokenSrv, ptr);	// Valor de token enviado por servidor
				else return TYP_SMS_ERR;
				//------------------------------------------------------------------------------------
				return TYP_SMS_CON;
			}
			else {
				strcpy(aux2, "SETRTC");
				if (strcmp(ptr, aux2) == 0){ 
					//------------------------ Procesa Puesta en hora de RTC -------------------------
					ptr = strtok(NULL, sepCh);
					if (ptr != NULL) hora = atoi(ptr);
					else return TYP_SMS_ERR;
					//
					ptr = strtok(NULL, sepCh);
					if (ptr != NULL) min = atoi(ptr);
					else return TYP_SMS_ERR;
					//
					ptr = strtok(NULL, sepCh);
					if (ptr != NULL) dia = atoi(ptr);
					else return TYP_SMS_ERR;
					//
					ptr = strtok(NULL, sepCh);
					if (ptr != NULL) mes = atoi(ptr);
					else return TYP_SMS_ERR;
					//
					ptr = strtok(NULL, sepCh);
					if (ptr != NULL) year = atoi(ptr+2);
					else return TYP_SMS_ERR;
					//
					ptr = strtok(NULL, sepCh);
					if (ptr != NULL) strcpy(tokenSrv, ptr);	// Valor de token enviado por servidor
					else return TYP_SMS_ERR;

					rtc_set_datetime(dia, mes, year, 1, hora, min);

					return TYP_SMS_RTC;
				}
				else {
					//
					strcpy(aux2, "INITMEM");
					if (strcmp(ptr, aux2) == 0){ 
						inicializa_eeprom(); 						// Inicializa memoria eeprom 
						ptr = strtok(NULL, sepCh);
					    if (ptr != NULL) strcpy(tokenSrv, ptr);	// Valor de token enviado por servidor
						
						return TYP_SMS_INI;
					}
					else {
						//
						strcpy(aux2, "CNUMSRV");
						strcpy(telaux, telnum);
						if (strcmp(ptr, aux2) == 0){
							ptr = strtok(NULL, sepCh);
							if (ptr != NULL) telnumaux = ptr; // Numero recibido
							else return TYP_SMS_ERR;
							//Escribir el tel num nuevo
							for (i = 0; i < SZ_TELEFONO; i++){
								//TODO write_eeprom(ADDR_TELEF1+i,*telnumaux);
								 telnumaux++;
							}
							//Leeer el num de tel nuevo
							for (i = 0; i < SZ_TELEFONO; i++){
								telnum[i] = '\0';
							//TODO	telnum[i] = read_eeprom(ADDR_TELEF1+i);
							}
							ptr = strtok(NULL, sepCh);
							if (ptr != NULL) strcpy(tokenSrv, ptr);	// Valor de token enviado por servidor
							return TYP_SMS_CSERV;
						}else{ 
							ptr = strtok(NULL, sepCh);
							if (ptr != NULL) strcpy(tokenSrv, ptr);	// Valor de token enviado por servidor
							return TYP_SMS_ERR;
						}
					}				
				}
			}
		}
	}
}

/************************************************************************************************
| void GenerateToken(void)
| Crea el token para enviar por la estacion
************************************************************************************************/
void GenerateToken(void)
{
	int valTkn;
	tokenEma[2] = 0;
	valTkn = atoi(tokenEma);
	valTkn++;
	if (valTkn > 99) valTkn = 0;
	sprintf(tokenEma, "%02d", valTkn);
}

//-----------------------------------------------------------------------------------------------
//
//------------------------------ Comienzo del programa principal --------------------------------
//
//-----------------------------------------------------------------------------------------------
void main()
{
	int i;
	// Desactivamos las interrupciones.
	disable_interrupts(GLOBAL);
	disable_interrupts(INT_rda);
	disable_interrupts(INT_EXT2);
	//------------------------ Inicializar puertos -------------------	
	set_tris_a(KIOPTOA);
    set_tris_b(KIOPTOB);
    set_tris_c(KIOPTOC);

	//Habilito el WatchDog
	setup_wdt(WDT_ON);
	// Configuramos el oscilador.
	setup_oscillator(OSC_NORMAL);    
	port_b_pullups(TRUE);
	// Configuramos el conversor AD.
	setup_adc(ADC_CLOCK_INTERNAL); 
	//setup_adc_ports(AN0_TO_AN1|VSS_VDD);     
    setup_adc_ports(AN0); 
	setup_timer_0(RTCC_INTERNAL|RTCC_DIV_32);        //1.0 s overflow
    setup_timer_3(T3_DISABLED|T3_DIV_BY_1);
    //
	if (input(INP_PREC) == 0){   		// Estado inicial
   		ext_int_edge(2, L_TO_H);  	// Configuro flanco de subida
  		flagInt2Edge = 0;    		// Indico que el siguiente flanco será de Subida
 	} 
	else {   
		ext_int_edge(2, H_TO_L);  	// Configuro flanco de Bajada
  		flagInt2Edge = 1;    		// Indico que el siguiente flanco será de Bajada
 	}	

	//----------------------- Inicializar variables ------------------
	runOk 		= 1;
	guia        = 1;
	address 	= 0;
	data 		= 0b10101010;
	temp		= 0;
	adc_val 	= 0;
	secUltMed   = 0;
	//TODO sensorHab 	= read_eeprom(ADDR_HAB_L); 
	//TODO timeMed 	= read_eeprom(ADDR_TMED_L) + (read_eeprom(ADDR_TMED_H) * 256);
	timeMedTmp 	= timeMed;
	//TODO timeChk		= read_eeprom(ADDR_TCHK_L) + (read_eeprom(ADDR_TCHK_H) * 256);
	timeChkTmp  = timeChk;
	ctePluv 	= KTE_PLUV; 	//ctePluv		= read_eeprom(ADDR_KPLUV_L) + (read_eeprom(ADDR_KPLUV_H) * 256);
	timeRestart = K_TIME_RESTART;
	
	//Escribir el telnum por defecto en la eeprom
	for (i = 0; i < SZ_TELEFONO; i++){
		 //TODO write_eeprom(ADDR_TELEF1+i, telnum[i]);
	}
							
	contData	= 0;
	ReadAlarmStruct();					// Lee alarmas guardadas en eeprom interna
	//----------------------- Flags ---------------------------------
	flagInt2Edge 	= 0;				// Indica flanco pluviometro
	memoryFull      = 0;                // Indica cuando no hay mas memoria
	startMeasure 	= 0;				// Indica tiempo de iniciar mediciones
	startCheck	 	= 0;				// Indica tiempo de iniciar chequeo interno
 	startRestart 	= 0;				// Indica tiempo de iniciar reinicio de modem
	countPrec 		= 0;				// Contador de precipitaciones
	smsMemSnd		= 0;				// Indica mensaje de memoria llena enviado
	llueveAhora     = 0; 		//Indica si esta lloviendo en este momento
	//----------------------------------------------------------------
	//------------------------ Habilitar Interrupciones --------------
	enable_interrupts(INT_TIMER0);
   	enable_interrupts(INT_EXT2);
    enable_interrupts(INT_RDA);
	clear_interrupt(INT_TIMER0);
	clear_interrupt(INT_EXT2);
	clear_interrupt(INT_RDA);
    enable_interrupts(GLOBAL);
	//------------------------ Inicializar dispositivos --------------
    rtc_init();
    sht_init();
    init_ext_eeprom();
    //output_bit(LED1, 1);
    //output_bit(LED2, 1);
	delay_ms(500);
    //output_bit(LED1, 0);
    //output_bit(LED2, 0);
    delay_ms(500);
	modulo_gsm_init();   		// inicio modem GSM
    delay_ms(90000);            // 90 segundos de yapa
	//output_bit(LED1, 1);
	borrar_sms_inicio();
    clear_var(smsText);	    	//Inicializo smsText
	reinicioOK();

    do{
		restart_wdt();
		if ((startMeasure) && (!memoryFull)){
			startMeasure = 0;
			Leer_sensores(1);					// LLena la estructura de medicion con los valores sin convertir
			StoreMed2EEPROM();					// Guarda en memoria los valores medidos junto a la fecha y hora
			construirTextoSms(smsOut,1);
			idLastSmsOut = EnviarSms(smsOut, telnum);		
		}
		else{
			if (memoryFull){
				//Ver accion en caso de memoria llena lo ideal seria un comando de inicializacion
				if (smsMemSnd == 0){
				    CurrDateTime();
					GenerateToken();
					clear_var(smsOut);
					sprintf(smsOut, "EST|%s|%s|MEMFULL|%ld|%c%c", dtFecha, dtHora, contData, tokenEma[0], tokenEma[1]);
					idLastSmsOut = EnviarSms(smsOut, telnum);
					smsMemSnd = 1;
				}
			}
		}
		if (startCheck){									// Verifica variables por alarmas
			CurrDateTime();
			float tempInt;
			startCheck = 0;
			CheckAlarmValues();
			if (tempInt != 0){									// Verifica estado del equipo
				GenerateToken();
				clear_var(smsOut);
				sprintf(smsOut, "EST|%s|%s|TEMP|%02.1f|%c%c", dtFecha, dtHora, tempInt,tokenEma[0], tokenEma[1]);
				idLastSmsOut = EnviarSms(smsOut, telnum);	// Alarma por temp. gabinete alta o baja
			}
		}
		if (startRestart){
			modulo_gsm_init();   		// inicio modem GSM
			delay_ms(90000);            // 90 segundos de yapa
			//output_bit(LED1, 1);
			startRestart = 0;
		}
		currSmsId = LeerSmsCmd(smsText);		// Lee sms y obtiene ID para su proceso posterior (borrado)
		if (currSmsId){									// Bandera que indica sms recibido
			resProc = ProcessMessage(smsText);
			clear_var(smsOut);
			switch (resProc){
				case TYP_SMS_INI:
				case TYP_SMS_RTC:
					sprintf(smsOut, "RTA|%04u|%c%c", resProc, tokenSrv[0], tokenSrv[1]);
					idLastSmsOut = EnviarSms(smsOut, telnum);
					break;
				case TYP_SMS_AA:
				case TYP_SMS_BA:
					WriteAlarmStruct();
					sprintf(smsOut, "RTA|%04u|%c%c", resProc, tokenSrv[0], tokenSrv[1]);
					idLastSmsOut = EnviarSms(smsOut, telnum);
					break;
				case TYP_SMS_CSERV:
					sprintf(smsOut, "RTA|%04u|%c%c", resProc, tokenSrv[0], tokenSrv[1]);
					idLastSmsOut = EnviarSms(smsOut, telaux);
					break;
				case TYP_SMS_CON:	
			        Leer_sensores(0);		//Lee las medidas de este momento
					construirTextoSms(smsOut,0);
					idLastSmsOut = EnviarSms(smsOut, telnum);
					break;
				case TYP_SMS_ERR:	
				default:	
					sprintf(smsOut, "RTA|%04u|%c%c", resProc, tokenSrv[0], tokenSrv[1]);
					idLastSmsOut = EnviarSms(smsOut, telnum);
					break;
			}
		}
    } while(runOk);
}
//---------------------------------
