#include "debug_utils.h"
unsigned int8 debug;

unsigned int1 isDebugON(void){
  if (debug == 1){
	return TRUE;
  }
 return FALSE;
}

void printDEBUGMessage(char *line1, char *line2) {
  char msgAux[16];
  i2cLCD_clear();
  sprintf(msgAux, "-> %s", line1);
  i2cLCD_message(msgAux);
  if (line2 != NULL) {
	i2cLCD_cursorPosition(1,0);
	sprintf(msgAux, "%s", line2);
	i2cLCD_message(msgAux);
  }
  delay_ms(2000);
}