#define  powerKeyOn                 output_high(SIM900_PWR)
#define  powerKeyOff                output_low(SIM900_PWR)
#define  BUFFER_SIZE                128
#define  STATUS_NOT_REG_NOT_SEARCH  0 
#define  STATUS_OK                  1
#define  STATUS_NOT_REG_SEARCHING   2
#define  STATUS_REG_DENY            3
#define  STATUS_OK_ROAMING          5
#define  MODEM_NOT_RESPOND          6

void clearRDABuffer(void);
int1 waitResponse(char *string, unsigned int16 timeout);
int1 modemStop(void);
int1 modemStopPWRKEY(void);
int1 isModemOK();
unsigned int8 getNetworkStatus(void);
char* getModemError(void);

volatile int8 buffer[BUFFER_SIZE + 1];
unsigned int8 buf_index,*ptr;
char telnum[] = "+5493416527105", timeout[] = "timeout", ok[] = "OK", err[] = "Err", smsText[SZ_SMSTXT], cellIdStr[5], lacStr[5];


int1 modemStop(void) {
  clearRDABuffer();
  //turn off modem
  fprintf(SIM900,"AT+CPOWD=1\r");
  delay_ms(15000);
  if (!waitResponse("NORMAL POWER DOWN", 3000)) {
    return FALSE;
  }
  if (isDebugON()) {
    printDEBUGMessage("AT+CPOWD", "OK");
  }
  return TRUE;
}

int1 modemStopPWRKEY(void) {
  clearRDABuffer();
  //turn off modem
  powerKeyOff;
  delay_ms(4000);
  powerKeyOn;
  if (!waitResponse("NORMAL POWER DOWN", 6000)) {
    return FALSE;
  }
  if (isDebugON()) {
    printDEBUGMessage("PWRKEY Off", "OK");
  }
  return TRUE;
}

int1 modemStart(void) {
  unsigned int8 res;
  
  clearRDABuffer();
  res = getNetworkStatus();
 
  if (isDebugON()) {
    char status[2];
	sprintf(status, "%d\0", res);
    printDEBUGMessage("[MODEM]", NULL);
    printDEBUGMessage("[Estado RED]", status);
  } 
  
  if (res == STATUS_OK || res == STATUS_OK_ROAMING)
    return TRUE; // modem registered correctly
  
  if (res != MODEM_NOT_RESPOND && res != STATUS_NOT_REG_NOT_SEARCH ) { //turn off modem
    if (isDebugON()) {
      printDEBUGMessage("[Apaga MODEM]", NULL);
    }
	  modemStop();
  }

  if (isDebugON()) {
    printDEBUGMessage("Iniciando...", NULL);
  } 
  //turn on modem with pwr key
  powerKeyOff;
  delay_ms(2000);
  powerKeyOn;
  delay_ms(5000);
  
  waitResponse("RDY", 20000);

  clearRDABuffer();
  fprintf(SIM900,"AT\r");

  if (!waitResponse(ok, 3000)){
   return FALSE;
  }

  if (isDebugON()) {
    printDEBUGMessage("[AT]", ok);
  }

  delay_ms(250);
  clearRDABuffer();
  fprintf(SIM900,"ATE0\r");

  if (!waitResponse(ok, 3000)){
   return FALSE;
  }

  if (isDebugON()) {
    printDEBUGMessage("[ATE0]",  ok);
  }

  delay_ms(250);
  clearRDABuffer();
  fprintf(SIM900,"AT+CMGF=1\r");

  if (!waitResponse(ok, 3000)){
   return FALSE;
  }

  if (isDebugON()) {
    printDEBUGMessage("[AT+CMGF]",  ok);
  }

  delay_ms(250);
  clearRDABuffer();
  fprintf(SIM900,"AT+CMEE=2\r");

  if (!waitResponse(ok, 3000)) {
   return FALSE;
  }

  if (isDebugON()) {
    printDEBUGMessage("[AT+CMEE]",  ok);
  }

  delay_ms(250);
  clearRDABuffer();
  fprintf(SIM900,"AT+CPMS=\"SM\"\r");

  if (!waitResponse(ok, 3000)){
    return FALSE;
  }  

  if (isDebugON()) {
    printDEBUGMessage("[AT+CPMS]", ok);
  }

  delay_ms(250);
  clearRDABuffer();
  fprintf(SIM900, "AT+CSCS=\"GSM\"\r");

  if (!waitResponse(ok, 3000)){
   return FALSE;
  }

  if (isDebugON()) {
    printDEBUGMessage("[AT+CSCS]",  ok);
  }

  delay_ms(250);
  clearRDABuffer();
  fprintf(SIM900, "AT+CSMP=17,167,0,241\r");

  if (!waitResponse(ok, 3000)){
   return FALSE;
  }

  if (isDebugON()) {
    printDEBUGMessage("[AT+CSMP]",  ok);
  }

  delay_ms(250);
  clearRDABuffer();
  fprintf(SIM900, "AT+CREG=2\r");

  if (!waitResponse(ok, 3000)){
   return FALSE;
  }
	
  if (isDebugON()) {
    printDEBUGMessage("[AT+CREG]",  ok);
  }

  if (!isModemOK())
    return FALSE;
  
  if (isDebugON()) {
    printDEBUGMessage("[MODEM]", "EN LINEA");
  }

  return TRUE;
}

int1 restartModem(void) {
	unsigned int8 i;
	modemStop();
    //Check 3 times in case error restarting
	for (i = 0; i < 3; i++) {
		if(modemStart()){
			return TRUE;
		}
	}
	// Try by PWRKEY
	modemStopPWRKEY();
	if (modemStart()) {
		return TRUE;
	}
	return FALSE;
}

void clearRDABuffer(void) {
  unsigned int8 erase;
  disable_interrupts(INT_RDA);
  erase = buf_index = 0;
  while (erase <= BUFFER_SIZE) {
    buffer[erase] = '\0';
    erase++;
  }
  enable_interrupts(INT_RDA);
}

int1 waitResponse(char *string, int16 timeout) {
  //Acá podríamos borrar el buffer... (pero tendríamos que meter refactor en otros lados)
  while (timeout) {
    if (strstr(buffer, string)) return TRUE;
    delay_ms(1);
    timeout--;
  }
  return FALSE;
}

void deleteSMS(void) {
  fprintf(SIM900,"AT+CMGD=1,0\r");
  delay_ms(250);
}

void deleteAllSMSInbox(void) {
  fprintf(SIM900,"AT+CMGDA=\"DEL ALL\"\r");
  delay_ms(250);
}

int1 sendSMS(char *sms, char *tel) {
  int1 ret_val = FALSE;
  unsigned int8 i;
  //Check 3 times in case error sending sms
  for (i = 0; i < 3; i++) {
    clearRDABuffer();
    fprintf(SIM900,"AT+CMGS=\"%s\"\r", tel);
    if (waitResponse(">", 3000)) {
      delay_ms(100);
      fprintf(SIM900, "%s", sms);
      delay_ms(1000);
      fputc(0x1A, SIM900);
	    delay_ms(2500);
      if (waitResponse("+CMGS:", 10000)) {
        ret_val = TRUE;
        break;
      } else {
        continue;
      }
    } else {
      // try again
      continue;
    }
  }
  if (isDebugON()) {
    printDEBUGMessage("RTA Envio :", (ret_val)? ok : err);
    if (!ret_val){
      printDEBUGMessage("[Error]", getModemError());
    }
  }
  return ret_val;
}

int1 readSMSInbox(char *sms) {
  char sepCh[2];
  char *ptr;
  char *ptr2;
  unsigned int8 i;

  // Refactor, usar "ALL" y traerse los numeros de los mensajes a leer y volar la mierda esa de 'guia'
  clearRDABuffer();

  delay_ms(500);
  fprintf(SIM900,"AT+CMGR=1,0\r");
  delay_ms(2000);

  if (waitResponse("UNREAD", 1000)) {
    if (waitResponse(telnum,1000)) {
      smsText[0] = '\0';
      strcpy(sepCh, "\"");
      ptr = strtok(buffer, sepCh);

      while ((ptr = strtok(NULL, sepCh)) != NULL)
        ptr2 = ptr;

      strcpy(sepCh, "\r");
      strtok(NULL, sepCh);
      ptr2 = ptr2 + 2;
      for (i = 0; i < SZ_SMSTXT; i++) {
        if ((ptr2[i] == '\r') || (ptr2[i] == '\n')) {
          ptr2[i] = 0;
          break;
        }
      }
      strcpy(sms,ptr2);
    }
    deleteSMS();
    return TRUE;
  } 
  return FALSE;
}

unsigned int8 getNetworkStatus() {
	unsigned int8 status;
	delay_ms(250);
	clearRDABuffer();
	fprintf(SIM900, "AT+CREG?\r"); 
	if (waitResponse(ok, 4000)) {
		unsigned int8 p, i;
		char registration[2];
		p = strchr(buffer, ',');
		registration[0] = *(p + 1);
		registration[1] = '\0';
		status = atoi(registration);
		if (status == STATUS_OK || status == STATUS_OK_ROAMING) {
		  p = p + 3; // skipeo carcteres no deseados
		  for (i = 0; i < 8; i++) {
		    if ( i <= 3) {
			    lacStr[i] = *(p + (i + 1));
			    if ( i == 3 ) 
			      p = p + 3; // skipeo carcteres no deseados
			  } else {
			    cellIdStr[i-4] = *(p + (i + 1));
			  }
		  }
		  cellIdStr[4] = lacStr[4] = '\0';
		}
		return status;
	}
	return MODEM_NOT_RESPOND; //modem is off
}

signed int8 getSignal(void) {
  clearRDABuffer();

  if (SIMULATION) {
    strcpy(buffer, "+CSQ: 12,99");
  } else {
    fprintf(SIM900, "AT+CSQ\r");
    if (!waitResponse(ok, 3000))
      return FALSE;
  }

  unsigned int8 p;
  char rssi[2];
  p = strchr(buffer, ' ');

  rssi[0] = *(p + 1);
  rssi[1] = (',' == *(p + 2)) ? '\0' : *(p + 2);

  return (99 == atoi(rssi)) ? 0 : (atoi(rssi) * 2 - 113);
}

char* getModemError(void){
  unsigned int8 error = strstr(buffer," ERROR: ");
  if ( error != NULL){
	  *(error + 23) = '\0';
	  error = error + 8;
  }
  return (error != NULL) ? error : timeout;
}

int1 isModemOK(void) {
  clearRDABuffer();
  unsigned int8 res = getNetworkStatus();
  if (res == STATUS_OK || res == STATUS_OK_ROAMING)
    return TRUE; // modem registered correctly
  return FALSE;
}