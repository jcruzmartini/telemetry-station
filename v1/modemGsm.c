void clear_buffer_gsm(void);
char  *gprs_response(char *s, int16 timeout);
char  *match_response(char *sms, char *s, int16 timeout);
void enviar_sms(void);
void SimPowerOff(void);
void wait_response_ok(int16 timeout, int times);
void wait_for_call_ready(void);

#define BUFFER_SIZE 255 
volatile int8 buffer[BUFFER_SIZE+1];
int8 buf_index = 0; 

//-------------------------------------------------------
//       MACROS
// NOTA : modificar en funcion al diseño de la placa
//-------------------------------------------------------
//Power Key
#define pk_on    output_high (PIN_B3)
#define pk_off   output_low (PIN_B3)
#define pk       input(PIN_B3)
#define fuente_on   output_high (PIN_C2)
#define fuente_off  output_low (PIN_C2)
#define fuente      input(PIN_C2)

// Punteros.
char *ptr;


//-----------------------------------------------------
//            LIBRERIAS
//-----------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <input.c>

//--------------------------------------------------------------------------
//  D E F I N I C I O N     D E  VARIABLES PARA MODULO GSM
//--------------------------------------------------------------------------

char ok[] = "OK";
char rd[] = "RDY";
char sms_noleido[] = { "UNREAD" };
char caracter[] = { ">" }; // caracter que me indica iniciar mensaje de texto

//int suspender;
int guia;
int done;

//#define telefono1 "+543415460866"   // OJO!! declarar tal cual esta, Numero de Telefono del SMS server

//-------------------------------------------------------
//  Funcion Delay 1 seg
//-------------------------------------------------------

#inline 
void delay_sec (void) 
{ 
   delay_ms(1000); 
} 


/*----------------------------------------------------------------------
| Funcion para apagar el modem
----------------------------------------------------------------------*/
void SimPowerOff(void)
{
	output_bit(SIM300_PWR, 0);
	delay_ms(1500);
	output_bit(SIM300_PWR, 1);
}

//--------------------------------------------------------
// INICIALIZACION DEL MODULO GSM
//--------------------------------------------------------
int modulo_gsm_init(void){
	//Inicio del modem segun DATASHEET SIMCOM 
	fuente_off;         // Apagamos la fuente del SIM300.
	pk_off;       		// PRWKEY=0;
	delay_ms(2000);		// Eperamos 2 seg
	fuente_on;			// Encendemos la fuente del SIM300.
	delay_ms(1000);      // Esperamos 1 seg
	pk_on; 				// PWRKEY=1;
	delay_ms(1500);		// Esperamos 1-1/2 seg.
	pk_off;				// PRWKEY=0;
	delay_ms(3000);		// Esperamos 3 seg.
	restart_wdt();
	wait_for_call_ready();
	clear_buffer_gsm();  // se borra buffer del modem
	fprintf(SIM300,"AT\r");
				
	while(gprs_response(ok,8000)==0) //  Esperando Respuesta del Modulo GSM
	{
	 fprintf(SIM300,"AT\r");
	}

	 //** ELIMINAR ECO **//   
	 clear_buffer_gsm();        // se borra buffer del modem
	 fprintf(SIM300,"ATE0\r");     // Elimina ECO
	 wait_response_ok(2000,5);
	
	 //** MODO TEXTO **//
	 clear_buffer_gsm();            // se borra buffer del modem
	 fprintf(SIM300,"AT+CMGF=1\r");    // Modo Texto
	 wait_response_ok(2000,5);
	
	 clear_buffer_gsm();         	 // se borra buffer del modem	
	 fprintf(SIM300,"AT+CMEE=2\r");    // Reporte de Errores en Modo Texto 
	 wait_response_ok(2000,5);
	 
	 clear_buffer_gsm();            // se borra buffer del modem
	 fprintf(SIM300,"AT+CPMS=\"ME\"\r"); // Selecciono la mememoria SIM para recibir y leer SMS.
	 wait_response_ok(2000,5);

	 clear_buffer_gsm();            			// se borra buffer del modem
	 fprintf(SIM300, "AT+CSCS=\"GSM\"\r");	//SMS text mode parameters
	 wait_response_ok(2000,5);
	 
	 clear_buffer_gsm();            			// se borra buffer del modem
	 fprintf(SIM300, "AT+CSMP=17,167,0,241\r");	//SMS text mode parameters
	 wait_response_ok(2000,5);
	
	 clear_buffer_gsm();            			// se borra buffer del modem
	return 1;
}


//-------------------------------------------------
//  BORRA EL BUFFER GSM
//-------------------------------------------------
void clear_buffer_gsm(void)  /*borra el buffer gsm */
{ 
  int erase=0;
  disable_interrupts(INT_RDA);
  
  buf_index = 0; 
  buffer[0] = 0; 
  while(erase!=BUFFER_SIZE) {
	buffer[erase] = '\0';
	erase++;
  }
  enable_interrupts(INT_RDA); 
} 

//---------------------------------------------------
// ESPERA LA RESPUESTA DEL MODULO
//---------------------------------------------------
void wait_response_ok(int16 timeout, int times){
	char tmOut = times;
	do {
		tmOut--;
	}while((gprs_response(ok,timeout) == NULL) && (tmOut != 0));
}

//-------------------------------------------
//FUNCION DE DEMORA A LA ESPERA DE OPERADORA
//-------------------------------------------
void wait_for_call_ready(void){
	disable_interrupts(INT_RDA);
    char tmOut = 10;
	do {
		tmOut--;
	} while((gprs_response(rd,5000) == NULL) && (tmOut != 0));
	enable_interrupts(INT_RDA);
}

//==================================================================
// funcion de comparacion de contenido buffer GSM
/* String que estamos buscando */ 
/* Tiempo de espera */ 
//==================================================================
char  *gprs_response(char *s, int16 timeout) 
{ 
  char  *p; 

  while (TRUE) 
  { 
    p = strstr(buffer, s); 
    if (p) 
    { 
       return (p); // salida de la funcion
    }      
    if (timeout) 
    {    
      timeout--; 
	  restart_wdt();
      if (!timeout) 
      { 
        return (NULL);    // timeout buffer
      } 
    } 
  } 
}  

//==================================================================
// funcion de comparacion de contenido de parametro sms
/* String que estamos buscando */ 
/* Tiempo de espera */ 
//==================================================================
char  *match_response(char *sms, char *s, int16 timeout) 
{ 
  char  *p; 
  while (TRUE) 
  { 
    p = strstr(sms, s); 
    if (p) 
    { 
       return (p); // salida de la funcion
    }      
    if (timeout) 
    {    
      timeout--; 
      if (!timeout) 
      { 
       return (NULL);    // timeout buffer
     } 
    } 
  } 
}  

//==================================================================
//  BORRAR SMS DEL MODEM
//==================================================================

int borrar_sms(void){
   int x;
   if (guia == 10){
		   for (x = 1; x <= 10; x++){
				fprintf(SIM300,"AT+CMGD=%d\r",x);
				gprs_response(ok,1000);
		   }
		guia=1;
		clear_buffer_gsm();
		return 1;
	}
	return 0;
}

//==================================================================
//  BORRAR TODOS LOS SMS  AL INICIO DEL MODEM
//==================================================================

void borrar_sms_inicio(void){
    int x;
	for (x = 1; x <= 50; x++){
		clear_buffer_gsm();
		fprintf(SIM300,"AT+CMGD=%d\r",x);
		gprs_response(ok,1000);
	}
}

