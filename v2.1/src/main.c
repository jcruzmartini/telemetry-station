/*
==================================================================
    Estacion Meteorologia Automatica TAU01 - Techner Argentina
==================================================================
*/
#define F_CLOCK  12
#define ADC_RES  16    //10 o 16 para PIC de 12 bits

#device PIC18F4523 *=16 ADC=ADC_RES WRITE_EEPROM=ASYNC WRITE_EEPROM=NOINT PASS_STRINGS=IN_RAM ICD=TRUE
#fuses PUT, NOLVP, HS, STVREN, MCLR, NODEBUG, PROTECT, WDT, WDT4096

#include <h\18f4523.h>
#include <stdlib.h>
#include <string.h>
#include <h\tau01.h>

int1 fillSensorsData(char scheduled);
void generateToken(void);
void clearVar(char *var);
float32 getPressure(void);
float32 getWindDirection(void);
float32 getWindSpeed(void);
float32 getGroundTemp(void);
float32 getRadiation(void);
float32 getWaterLevel(void);
float32 getVBat(void);
void sendRestartSMS(void);
void buildSMSText(char *sms, char alarmScheduled);
void concatVariableSeparator(char *sms);
void checkButtons(void);
void doDebugLogic(void);
void displayStatus(void);
void checkSensorsEnabled(void);
void checkAlarms(void);
void writeSensorDataInLCD(char *text, char *format, float value);
unsigned int16 getADCValue(unsigned int8 channel, unsigned int8 qty);

char sepVar[] = "*";
int1 startMeasure, startCheck, startRestart, timer0Overflow;
unsigned int8 resProc;
unsigned int16 countPrec, timeMed, timeMedTmp, timeChkTmp, timeChk,timeRestartTmp, timeRestart, contData, acumPrec, sensorHab;
char tokenSrv[3], tokenEma[3], smsOut[SZ_SMSTXT], charFloat1[10], msg[16];

struct medSrt {
  int32 dateTime;
  float32 humidity;
  float32 tempExt;
  float32 precipitation;
  float32 pressure;
  float32 wDirection;
  float32 wSpeed;
  float32 tempGnd;
  float32 humGnd;
  float32 battery; 
  float32 radiation; 
  float32 other; 
};    // 46 Bytes

struct medSrt dataMed;

struct alarmaSt {
  unsigned int8 activa;
  unsigned int8 variable;
  float32 max_value;
  float32 min_value;
  unsigned int8 cnValidMin;
  unsigned int8 cnValidMax;
  unsigned int8 sleep;
};    // 13 bytes

struct alarmaSt alarm[5];    // 65 bytes

void write_eeprom_16bits(int address, int16* val){
  int pLow, pHigh;
  pLow = val;
  pHigh = val>>8;
  write_eeprom(address,pHigh);
  delay_ms(12);
  ++address;
  write_eeprom(address,plow);
  delay_ms(12);
}

long read_eeprom_16bits(int address){
  int pLow, pHigh;
  long result;
  pHigh = read_eeprom(address);
  ++address;
  pLow = read_eeprom(address);
  result=(pHigh<<8);
  result+=pLow;
  return result; 
}

void ReadAlarmStruct(void) {
  long addtmp;
  char *p;
  int i;
  p = &alarm[0];
  addtmp = AA_ADDR;
  for (i = 0; i < SZ_ALRMSTR; i++) {
    *p = read_eeprom(addtmp);
    p++;
    addtmp++;
  }
}

void WriteAlarmStruct(void) {
  long addtmp;
  char *p;
  int i;
  p = &alarm[0];
  addtmp = AA_ADDR;
  for (i = 0; i < SZ_ALRMSTR; i++) {
    write_eeprom(addtmp, *p);
    p++;
    addtmp++;
  }
}

float getValueToCompare(int8 var){
  switch(var){
    case 'T':       // Temperatura  
      return dataMed.tempExt;
    case 'H':       // Humedad
      return dataMed.humidity;             
    case 'V':       // Vel. Viento
      return dataMed.wSpeed;             
    case 'D':       // Dir. Viento
      return dataMed.wDirection;             
    case 'S':       // Temperatura de suelo
      return dataMed.tempGnd;             
    case 'L':       // Lluvia
      return dataMed.precipitation;         
    case 'P':       //Pŕesion
      return dataMed.pressure;         
    case 'W':       //Humedad de suelo
      return dataMed.humGnd;     
    case 'R':       //Radiacion
      return dataMed.radiation;   
    case 'F':       //Freatimetro
      return dataMed.other;      
    break;
  }  
}

void sendAlarmSMS(unsigned int8 variable, float value) {
  char auxSms[4];
  generateToken();
  sprintf(auxSms, "%4.1f", value);
  sprintf(smsOut, "A|%c|%s|%c%c", variable, auxSms, tokenEma[0], tokenEma[1]);
  if (isDebugON()){
    printDEBUGMessage("Envia Alarma", smsOut);
  }
  int1 resp = sendSMS(smsOut,telnum);
  if (isDebugON()) {
  	printDEBUGMessage("RTA Envio: ", (resp)? "OK" : "Err");
  }
}

void checkAlarms(void) {
  unsigned int8 i;
  int1 inAlarm = 0;
  fillSensorsData(0);
  for (i = 0; i < 5; i++) {
    if (alarm[i].activa != 1)
      continue;  
    if (alarm[i].sleep != 0){
      alarm[i].sleep--;
      continue;
    }
       
    float valueToCompare = getValueToCompare(alarm[i].variable);
    if (valueToCompare > alarm[i].max_value){
		  alarm[i].cnValidMax++;
      alarm[i].cnValidMin = 0;
		  inAlarm = 1;
	  } else {
  		if (valueToCompare < alarm[i].min_value){
  			alarm[i].cnValidMin++;
        alarm[i].cnValidMax = 0;
  			inAlarm = 1;
  		} else {
  			alarm[i].cnValidMax = alarm[i].cnValidMin = 0;
  		}
	  }
  	if (inAlarm) {
  		if (alarm[i].cnValidMax != MAX_CNALARM && alarm[i].cnValidMin != MAX_CNALARM) {
        continue;
      } 
      alarm[i].cnValidMax = alarm[i].cnValidMin = 0;
      alarm[i].sleep = ALARM_SLEEP;
      sendAlarmSMS(alarm[i].variable, valueToCompare);
  	}
  }
}

void sendRestartSMS(void) {
  delay_ms(1000);
  generateToken();
  CurrDateTime();
  sprintf(smsOut, "EST|%s|%s|ON|%i-%s-%s|%c%c", dtFecha, dtHora, restart_cause(), lacStr, cellIdStr, tokenEma[0], tokenEma[1]);
  i2cLCD_message("Registrandose...");
  int1 respOK = sendSMS(smsOut,telnum);
  if (respOK ) {
	  i2cLCD_cursorPosition(1, 11);
      i2cLCD_message("[OK]");
  } else {
	  i2cLCD_cursorPosition(1, 11);
    i2cLCD_message("[Err]");
  }
  delay_ms(1000);
}

void concatVariableSeparator(char* sms) {
  if (strlen(sms) != 0)
    strcat(sms, sepVar);
}

int1 sensorEnabled(unsigned int8 sensorBit) {
  return bit_test(sensorHab, sensorBit);
}

void buildSMSText(char* sms, char scheduled) {
  char auxVar[SZ_SMSTXT];
  clearVar(auxVar);

  if (sensorEnabled(TEMP_HUM_S)) {
    sprintf(charFloat1, "T:%2.1f",dataMed.tempExt);
    charFloat1[6] = 0;
    strcat(auxVar,charFloat1);
    concatVariableSeparator(auxVar);
    sprintf(charFloat1, "H:%2.1f",dataMed.humidity);
    charFloat1[6] = 0;
    strcat(auxVar,charFloat1);
  }
  if (sensorEnabled(RAINFALL_S)) {
    concatVariableSeparator(auxVar);
    sprintf(charFloat1, "L:%2.1f", dataMed.precipitation);
    charFloat1[6] = 0;
    strcat(auxVar,charFloat1);
  }
  if (sensorEnabled(PRESSURE_S)) {
    concatVariableSeparator(auxVar);
    sprintf(charFloat1, "P:%4.1f", dataMed.pressure);
    charFloat1[8] = 0;
    strcat(auxVar,charFloat1);
  }
  if (sensorEnabled(TEMP_GND_S)) {
    concatVariableSeparator(auxVar);
    sprintf(charFloat1, "S:%2.1f", dataMed.tempGnd);
    charFloat1[6] = 0;
    strcat(auxVar,charFloat1);
  }
  if (sensorEnabled(HUM_GND_S)) {
    concatVariableSeparator(auxVar);
    sprintf(charFloat1, "W:%2.1f", dataMed.humGnd);
    charFloat1[6] = 0;
    strcat(auxVar,charFloat1);
  }
  if (sensorEnabled(RADIATION_S)) {
    concatVariableSeparator(auxVar);
    sprintf(charFloat1, "R:%4.2f", dataMed.radiation);
    charFloat1[9] = 0;
    strcat(auxVar,charFloat1);
  }
  if (sensorEnabled(WIND_S)) {
    concatVariableSeparator(auxVar);
    sprintf(charFloat1, "V:%3.1f", dataMed.wSpeed);
    charFloat1[7] = 0;
    strcat(auxVar,charFloat1);
    concatVariableSeparator(auxVar);
    sprintf(charFloat1, "D:%3.0f", dataMed.wDirection);
    charFloat1[7] = 0;
    strcat(auxVar,charFloat1);
  }
  if (sensorEnabled(OTHER_S)) {
    concatVariableSeparator(auxVar);
    sprintf(charFloat1, "F:%2.2f", dataMed.other);
    charFloat1[7] = 0;
    strcat(auxVar,charFloat1);
  }
  
  //Nivel Bateria
  if (scheduled) {
	  concatVariableSeparator(auxVar);
	  sprintf(charFloat1, "B:%2.1f", dataMed.battery);
	  charFloat1[6] = 0;
	  strcat(auxVar,charFloat1);
  }
  
  clearVar(sms);
  if (scheduled) {
    generateToken();
    sprintf(sms, "D|%s|%s|%s|%c%c",dtFecha, dtHora, auxVar, tokenEma[0], tokenEma[1]);
  } else {
    sprintf(sms, "R|%s|%s|%s|%c%c",dtFecha, dtHora, auxVar, tokenSrv[0], tokenSrv[1]);
  }
}

unsigned int16 getADCValue(unsigned int8 channel, unsigned int8 qty) {
  int8 i;  
  unsigned int32 adc_val = 0;
  set_adc_channel(channel);
  delay_ms(10);
  for (i = 0; i < qty; i++) {
    read_adc(ADC_START_ONLY);
    while(!adc_done());
    adc_val = adc_val + read_adc(ADC_READ_ONLY);
  }
  #if ADC_RES == 10
	return (adc_val)/qty;
  #elif ADC_RES == 16
	return (adc_val >> 4)/qty;
  #endif
}

float getPressure(void) {
  float pressure = 0;
  int16 adc_val = getADCValue(2,64);
  pressure = ((adc_val * KTEMPX) + KTEMPX2) * 10;
  return pressure;
}

float getRadiation(void) {
  float radiation = 0;
  int16 adc_val = getADCValue(3,10);
  radiation = (adc_val * ADC_STEP * 1000000)/KTE_RADIATION;
  return radiation;
}

float getWaterLevel(void) {
  float water = 0;
  int16 adc_val = getADCValue(1,10);
  water = ((adc_val * ADC_STEP * 1000) - OFFSET_WATER_LEVEL)/WATER_LEVEL_1_M; //in meters
  water = KTE_WATER_LEVEL - water; 
  return water;
}

float getWindDirection(void) {
  float direction = 0;
  int16 adc_val = getADCValue(4,1);
  direction = adc_val * 360.0; //In degrees
  direction = direction / MAX_ADC_RESOLUTION;  
  return direction;
}

float getWindSpeed(void) {
  int8 pulses = 0;
  int16 timeout = WIND_SPEED_TIME * 1000; // in ms
  while (timeout) {
    if (input(WIND_SPEED)){
      pulses++;
    }
	delay_ms(1);    
    timeout--;
  }
  float speed = (pulses * WIND_SPEED_PREC)/WIND_SPEED_TIME; // in m/s
  return speed * 3.6; //in km/h
}

float getGroundTemp(void) {
	//if ( DS1820_FindFirstDevice() ) {
	// return DS1820_GetTempFloat();
	//}
	//return DS1820_CONN_ERROR;
	return ds1820_read();
}

#int_TIMER0
void  TIMER0_isr(void) {
  timeMedTmp--;
  if (0 == timeMedTmp) {
    timeMedTmp = timeMed;
    startMeasure = 1;
  }
  timeChkTmp--;
  if (0 == timeChkTmp) {
    timeChkTmp = timeChk;
    startCheck = 1;
  }
  timeRestartTmp--;
  if (0 == timeRestartTmp) {
    timeRestartTmp = timeRestart;
    startRestart = 1;
  }
  timer0Overflow = 1;
}

#int_EXT
void  EXT_isr(void) {
  countPrec++;
}

#int_RDA
void RDA_isr(void) {
  if (kbhit(SIM900)) {
    if (buf_index >= BUFFER_SIZE) {
      fgetc(SIM900);
    } else {
      buffer[buf_index] = fgetc(SIM900);
      buffer[buf_index + 1] = "\0";
      buf_index++;
    }
  }
}

int1 fillSensorsData(char sheduled) {
  dataMed.dateTime = CurrDateTime();
  if (sensorEnabled(TEMP_HUM_S)){
    #if TEMP_HUM_SENS_TYPE == 1
      sht_rd(dataMed.tempExt, dataMed.humidity);
    #elif TEMP_HUM_SENS_TYPE == 2
	  dht22_read(dataMed.humidity, dataMed.tempExt);
    #endif
  } 
  if (sensorEnabled(RAINFALL_S)) {
    if (sheduled) {
      dataMed.precipitation = ((float32)(countPrec * KTE_PLUV));
	  if (contData > PRE_ACUM){
        acumPrec = countPrec;
		contData = 1;
      } else {
		acumPrec = acumPrec + countPrec;
	  }
      countPrec = 0;
    } else {
        dataMed.precipitation = ((float32)((acumPrec + countPrec) * KTE_PLUV));
    }
  }
  if (sensorEnabled(PRESSURE_S)) {
    dataMed.pressure = getPressure();
  }
  if (sensorEnabled(RADIATION_S)) {
    dataMed.radiation = getRadiation();
  }
  if (sensorEnabled(WIND_S)) {
    dataMed.wDirection = getWindDirection();  
    dataMed.wSpeed = getWindSpeed();
  }
  if (sensorEnabled(TEMP_GND_S)) {
    dataMed.tempGnd = getGroundTemp();
  }
  if (sensorEnabled(OTHER_S)) {
    dataMed.other = getWaterLevel();
  }
  dataMed.battery = getVBat();

  return TRUE;
}

void clearVar(char* sms) {
  int erase = 0;
  while(erase < SZ_SMSTXT) {
    sms[erase] = '\0';
    erase++;
  }
}

int ProcessMessage(char *msg) {
  char aux[30], sepCh[2], *ptr;
  byte hora, min, dia, mes, year;
  byte typeAlrm, i, done;
  float maxAlrm, minAlrm;

  memset(aux, 0, sizeof(aux));
  strcpy(aux, msg);
  strcpy(sepCh, "|");
  ptr = strtok(aux, sepCh);

  if (strcmp(ptr, "AA") == 0) {
    ptr = strtok(NULL, sepCh);
    if (ptr != NULL) {
       typeAlrm = *ptr;
    } else {
       return TYP_SMS_ERR;
    }
    ptr = strtok(NULL, sepCh);
    if (ptr != NULL) {
       maxAlrm = atof(ptr);
    } else {
       return TYP_SMS_ERR;
    }
    ptr = strtok(null, sepCh);
    if (ptr != NULL) {
       minAlrm = atof(ptr);
    } else {
       return TYP_SMS_ERR;
    }
    ptr = strtok(null, sepCh);
    if (ptr != NULL) {
       strcpy(tokenSrv, ptr);
    } else {
      return TYP_SMS_ERR;
    }
	int8 id = -1;
    for (i = 0; i < 5; i++) {
      if (alarm[i].variable == typeAlrm ) {
	    id = i;
		break;
	  }
    }
	if (id == -1) {
		for (i = 0; i < 5; i++) {
		  if (alarm[i].activa != 1) {
			id = i;
			break;
		  }
		}
	}		
	if (id == -1) {
		return TYP_SMS_ERR; //ERROR: no queda lugar disp para alarmas
	}
    alarm[id].activa = 1;
    alarm[id].variable = typeAlrm;
    alarm[id].max_value = maxAlrm;
    alarm[id].min_value = minAlrm;
    alarm[id].cnValidMin = 0;
    alarm[id].cnValidMax = 0;
    alarm[id].sleep = 0;

    return TYP_SMS_AA;
	
  } else if (strcmp(ptr, "BA") == 0) {
    ptr = strtok(null, sepCh);
    if (ptr != NULL)
      typeAlrm = *ptr;
    else
      return TYP_SMS_ERR;
    ptr = strtok(null, sepCh);
    if (ptr != NULL)
      strcpy(tokenSrv, ptr);
    else
      return TYP_SMS_ERR;
    done = 0;
    for (i = 0; i < 5; i++) {
      if (alarm[i].variable == typeAlrm) {
        alarm[i].activa    = 0;
        alarm[i].cnValidMin   = 0;
        alarm[i].cnValidMax   = 0;
        alarm[i].sleep = 0;
        done = 1;
        break;
      }
    }
    if (!done)
      return TYP_SMS_ERR;
    return TYP_SMS_BA;

  } else if (strcmp(ptr, "INFO") == 0) {
    ptr = strtok(null, sepCh);
    if (ptr != NULL)
      strcpy(tokenSrv, ptr);
    else
      return TYP_SMS_ERR;
    return TYP_SMS_CON;

  } else if (strcmp(ptr, "SETRTC") == 0) {
    ptr = strtok(NULL, sepCh);
    if (ptr != NULL)
      hora = atoi(ptr);
    else
      return TYP_SMS_ERR;
    ptr = strtok(NULL, sepCh);
    if (ptr != NULL)
      min = atoi(ptr);
    else
      return TYP_SMS_ERR;
    ptr = strtok(NULL, sepCh);
    if (ptr != NULL)
      dia = atoi(ptr);
    else
      return TYP_SMS_ERR;
    ptr = strtok(NULL, sepCh);
    if (ptr != NULL)
      mes = atoi(ptr);
    else
      return TYP_SMS_ERR;
    ptr = strtok(NULL, sepCh);
    if (ptr != NULL)
        year = atoi(ptr + 2);
    else
      return TYP_SMS_ERR;
    ptr = strtok(NULL, sepCh);
    if (ptr != NULL)
      strcpy(tokenSrv, ptr);
    else
      return TYP_SMS_ERR;
    rtc_set_datetime(dia, mes, year, 1, hora, min);
    return TYP_SMS_RTC;

  } else if (strcmp(ptr, "CSH") == 0) {
    int16 sensorHabTemp;
    signed int8 k;
    ptr = strtok(NULL, sepCh);
    for (k = 0; k <= MAX_CONF_SENS; k++) {
      if ('1' == *(ptr + k))
        bit_set(sensorHabTemp, (MAX_CONF_SENS - k));
      else if ('0' == *(ptr + k))
        bit_clear(sensorHabTemp, (MAX_CONF_SENS - k));
      else
        return TYP_SMS_ERR;
    }

    write_eeprom_16bits(CSH_ADDR, sensorHabTemp);
    sensorHab = sensorHabTemp;

    ptr = strtok(NULL, sepCh);
    if (ptr != NULL)
      strcpy(tokenSrv, ptr);
    return TYP_SMS_CSH;
  } else if (strcmp(ptr, "RESTART") == 0) {
    return TYP_SMS_RESTART;
  } else {
    ptr = strtok(NULL, sepCh);
    if (ptr != NULL)
      strcpy(tokenSrv, ptr);
    return TYP_SMS_ERR;
  }
}

void generateToken(void) {
  int valTkn;
  tokenEma[2] = 0;
  valTkn = atoi(tokenEma);
  valTkn++;
  if (valTkn > 99)
    valTkn = 0;
  sprintf(tokenEma, "%02d", valTkn);
}

float32 getVBat(void) {
  int16 adc_val = getADCValue(10,1);
  return ((adc_val * VCC * VOLT_DIVIDER)/MAX_ADC_RESOLUTION);
}


void writeSensorDataInLCD(char *text, char *formatedValue){
  i2cLCD_message(text);
  i2cLCD_cursorPosition(1,0);
  i2cLCD_message(formatedValue);
  delay_ms(3000);
  i2cLCD_clear();
}

void displayStatus(void) {
  i2cLCD_backlight(1);
  i2cLCD_clear();
  i2cLCD_message("Consultando... ");  
  fillSensorsData(0); 
  delay_ms(1000);  
  i2cLCD_clear();
  
  signed int8 signalQuality;
  signalQuality = getSignal();

  if (signalQuality) {
    sprintf(msg, "%04d dBm", signalQuality);
  } else {
    msg = "Error";
  }
  writeSensorDataInLCD("Nivel de Senal: ", msg);
  writeSensorDataInLCD("Estado RED GSM: ", isModemOK() ? ok : err);

  sprintf(msg, "%2.1f Volts", dataMed.battery);
  writeSensorDataInLCD("Nivel Bateria: ", msg);
  
  if (sensorEnabled(OTHER_S)) {
    sprintf(msg, "%2.2f m", dataMed.other);
    writeSensorDataInLCD("Freatimetro: ", msg);
  }
  if (sensorEnabled(TEMP_HUM_S)) {
    sprintf(msg, "%2.1f C", dataMed.tempExt);
    writeSensorDataInLCD("Temperatura: ", msg);
	sprintf(msg, "%2.1f porciento", dataMed.humidity);
    writeSensorDataInLCD("Humedad: ", msg);
  }

  if (sensorEnabled(RAINFALL_S)) {
  	sprintf(msg, "%2.1f mm", dataMed.precipitation);
    writeSensorDataInLCD("Precip. (24h): ", msg);
  }

  if (sensorEnabled(PRESSURE_S)) {
    sprintf(msg, "%4.1f hPa", dataMed.pressure );
    writeSensorDataInLCD("Presion: ", msg );
  }

  if (sensorEnabled(RADIATION_S)) {
    sprintf(msg, "%4.2f W/m2", dataMed.radiation );
    writeSensorDataInLCD("Radiacion: ", msg );
  }

  if (sensorEnabled(TEMP_GND_S)) {
    sprintf(msg, "%2.1f C", dataMed.tempGnd);
    writeSensorDataInLCD("Temp. Suelo: ", msg);
  }

  if (sensorEnabled(HUM_GND_S)) {
    sprintf(msg, "%2.1f porciento", dataMed.humGnd);
    writeSensorDataInLCD("Hum. Suelo: ", msg);
  }
  if (sensorEnabled(WIND_S)) {
    sprintf(msg, "%3.1f km/h", dataMed.wSpeed);
    writeSensorDataInLCD("Vel. Viento: ", msg);
    sprintf(msg, "%3.1f grados", dataMed.wDirection);
	 writeSensorDataInLCD("Dir. Viento: ", msg);
  }
}

void checkButtons(void) {
  if (!input(BUTTON2)) {
    int16 b_val = getADCValue(7,1); //chequeo boton PRG analogico
	if (b_val < 600){
	    i2cLCD_backlight(1);
		doDebugLogic();
	} else {
		displayStatus();
	}
	while (!input(BUTTON2))
      restart_wdt();
  }
}

void doDebugLogic(void) {
  i2cLCD_clear();
  i2cLCD_message("Modo DEBUG ");
  if (debug != 1){
	  i2cLCD_message(" [ON]");
	  write_eeprom(DEBUG_ADDR,1);
	  debug = 1;
  } else {
	  i2cLCD_message("[OFF]");
	  write_eeprom(DEBUG_ADDR,0);
	  debug = 0;
  }
  delay_ms(2000);  
}

void checkSensorsEnabled(void) {
	unsigned int16 sensorHabTemp = read_eeprom_16bits (CSH_ADDR);
	if ( sensorHabTemp == 0xFFFF) {
		sensorHabTemp = 0x0000;
		bit_set(sensorHabTemp, TEMP_HUM_S);
		bit_set(sensorHabTemp, RAINFALL_S);
		write_eeprom_16bits (CSH_ADDR, sensorHabTemp);
	}
    sensorHab = sensorHabTemp;
}

void main() {
  disable_interrupts(GLOBAL);

  set_tris_a(TRIS_A);
  set_tris_b(TRIS_B);
  
  setup_wdt(WDT_ON);
  setup_oscillator(OSC_NORMAL);

  #if F_CLOCK == 10
	setup_adc(ADC_CLOCK_DIV_8);  //Reloj 10 mhz -> DIV_8  // Dato obtenido de datasheet tabla 19.1.  
  #elif F_CLOCK == 12
	setup_adc(ADC_CLOCK_DIV_16); // Reloj 12 mhz -> DIV_16 // Dato obtenido de datasheet tabla 19.1. 
  #endif

  setup_adc_ports(AN0_TO_AN10);

  setup_timer_0(T0_INTERNAL|T0_DIV_256);
  setup_timer_3(T3_DISABLED|T3_DIV_BY_1);

  timeMed = ((60 * REPORT_MINUTES) / T0OF_SECS);
  timeMedTmp = timeMed;
  timeChk = ((60 * ALARM_MINUTES) / T0OF_SECS);
  timeChkTmp = timeChk;
  timeRestart = ((60 * RESTART_MINUTES) / T0OF_SECS);
  timeRestartTmp = timeRestart;

  checkSensorsEnabled();
  debug = read_eeprom(DEBUG_ADDR);
  acumPrec = contData = 0;
  
  ReadAlarmStruct();

  startMeasure = 0;  // Indica tiempo de iniciar mediciones
  startCheck = 0;    // Indica tiempo de iniciar chequeo interno
  startRestart = 0;  // Indica tiempo de iniciar reinicio del modem
  countPrec = 0;     // Contador de precipitaciones

  rtc_init();
  
  if (sensorEnabled(TEMP_HUM_S)) {
    #if TEMP_HUM_SENS_TYPE == 1
      sht_init();
    #elif TEMP_HUM_SENS_TYPE == 2
	  dht22_start();
    #endif
  }

  delay_ms(1500);

  i2cLCD_init();
  i2cLCD_backlight(1);
  i2cLCD_message("Iniciando  TAU01");
  delay_ms(1500);

  clear_interrupt(INT_TIMER0);
  clear_interrupt(INT_EXT);
  clear_interrupt(INT_RDA);
  clearRDABuffer();
  enable_interrupts(INT_TIMER0);
  enable_interrupts(INT_EXT);
  enable_interrupts(INT_RDA);
  enable_interrupts(GLOBAL);
  
  int16 b_val = getADCValue(7,1); //chequeo boton PRG analogico 
  if (b_val < 600) {
	  doDebugLogic();
  }
  
  if (!SIMULATION) {
    if (!isDebugON()) {
	   i2cLCD_cursorPosition(1,0);
	   i2cLCD_message("Modem      ");
    }
    if (!modemStart()) {
	    if (!isDebugON()) {
	      i2cLCD_message("[Err]");
	    } else {
		   printDEBUGMessage("[Error]", getModemError());
		}
	    modemStop();
      delay_ms(5000);
      reset_cpu();
    }
	  if (!isDebugON()) {
	    i2cLCD_message(" [OK]");
	  }
    delay_ms(3000);
    i2cLCD_clear();

    deleteAllSMSInbox();
    clearVar(smsText);
    sendRestartSMS();
  }

  unsigned int8 hours, minutes, seconds;

  while (TRUE) {
    timer0Overflow = 0;
    while (!timer0Overflow) {
      if (!input(BUTTON2)) {
        delay_ms(30);
        i2cLCD_clear();
        checkButtons();
        delay_ms(500);
      }
      restart_wdt();
    }

    rtc_get_time(hours, minutes, seconds);
    i2cLCD_clear();

    i2cLCD_message("TAU-01");
    sprintf(msg, "%02u:%02u", hours, minutes);
    i2cLCD_cursorPosition(1,0);
    i2cLCD_message(msg);

    if (startMeasure) {
      contData++;
      startMeasure = 0;
      fillSensorsData(1);
      buildSMSText(smsOut,1);
      sendSMS(smsOut,telnum);
    } 

    if (startCheck) {
      startCheck = 0;
      CurrDateTime();
      checkAlarms();
    }
	
  	if (startRestart) {
  	   startRestart = 0;
       i2cLCD_clear();
  	   i2cLCD_backlight(1);
       i2cLCD_message("[Estado Red GSM]");
  	   i2cLCD_cursorPosition(1,0);
  	   if (isModemOK()) {
  	      i2cLCD_message("            [OK]");
  	   } else {
  		  i2cLCD_message("           [Err]");
  		  restartModem();
  		  delay_ms(3000);
  	   }
       i2cLCD_clear();
  	}

    if (readSMSInbox(smsText)) {
	    int1 resp;
	    if (isDebugON()) {
        printDEBUGMessage("Nuevo SMS", smsText);
      }
      resProc = ProcessMessage(smsText);
      clearVar(smsOut);
      switch (resProc) {
        //Default cases for: TYP_SMS_INI / TYP_SMS_RTC / TYP_SMS_CSH / TYP_SMS_CSERV / TYP_SMS_ERR
        default:           
          sprintf(smsOut, "RTA|%04u|%c%c", resProc, tokenSrv[0], tokenSrv[1]);
          resp = sendSMS(smsOut,telnum);
          break;
        case TYP_SMS_AA:
        case TYP_SMS_BA:
          WriteAlarmStruct();
          sprintf(smsOut, "RTA|%04u|%c%c", resProc, tokenSrv[0], tokenSrv[1]);
          resp = sendSMS(smsOut,telnum);
          break;
        case TYP_SMS_RESTART:
          sprintf(smsOut, "RTA|%04u|%c%c", resProc, tokenSrv[0], tokenSrv[1]);
          resp = sendSMS(smsOut,telnum);
          delay_ms(2000);
          reset_cpu();
          break;
        case TYP_SMS_CON:
          fillSensorsData(0);
          buildSMSText(smsOut,0);
          resp = sendSMS(smsOut,telnum);
          break;
      }
	    if (isDebugON()) {
    	  printDEBUGMessage("Envio SMS", smsOut);
  	    printDEBUGMessage("RTA Envio: ", (resp)? "OK" : "Err");
      }
    }
    i2cLCD_backlight(0);
	restart_wdt();
  }
}
