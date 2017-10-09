typedef struct dateSt {         // estructura para la conversión de fecha
   char  hour;
   char  min;
   char  sec;
   char  day;
   char  mon;
   char  dow;
   long  year;
} TDateSt;

const long  mes[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
const char S2Ddays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

char	dtFecha[10], dtHora[10];

/*----------------------------------------------------------------------------------------------
| Date2Sec(TDateSt time) Convierte la fecha pasada en la estructura en segundos
| Parametro: 'time' : estructura tipo TDateSt
| RETURN VALUE: unsigned long (segundos)
-----------------------------------------------------------------------------------------------*/
int32  Date2Sec(TDateSt *time)
{
   int32  t, xt;

   xt = 365L * (int32)(time->year - 1970) + (int32)(mes[time->mon-1] + time->day - 1);
   xt += (int32)((time->year - 1969) >> 2);
   if ((time->mon > 2) && ((time->year & 3) == 0)) xt++;
   t = (int32)(time->sec) + 60L * ((int32)(time->min) + 60L * (int32)(time->hour + 24 * xt));
   return t;
}
//-----------------------------------------------------------------------------------------------------

/*------------------------------------------------------------------------------------------------------
|  Sec2Date(unsigned long x, TDateSt *pD)
|  Convierte la fecha expresada en segundos a día, mes, año, día de la semana, horas, minutos y segundos.
|  Parametro: 'x'   : fecha en segundos
|   		  'pD'  : puntero a una estructura tipo TDateSt donde se devuelve el resultado
|  RETURN VALUE:  ninguno
------------------------------------------------------------------------------------------------------*/
void Sec2Date(int32 x, TDateSt *pD)
{
   int32 iMin, iHor, iDay;
   long   hpery;
   int32 i, cumdays;

   iMin = x / 60;
   pD->sec = (char)(x - (60 * iMin));
   iHor = iMin / 60;
   pD->min = (char)(iMin - 60 * iHor);
   iDay = iHor / 24;
   pD->hour = (char)(iHor - 24 * iDay);
   i = (int32)(iHor / (1461L * 24L));
   cumdays = 1461L * i;
   pD->year = (long)(i << 2);
   pD->year += (long)1970;
   iHor %= 1461L * 24L;
   for (;;) {
     hpery = 365 * 24;
     if ((pD->year & 3) == 0) hpery += 24;
     if (iHor < (int32)hpery) break;
     cumdays += hpery / 24;
     pD->year++;
     iHor -= hpery;
   }
   iHor /= 24;
   cumdays += (long)iHor + 4;
   pD->dow = (char)((cumdays % 7)+1);
   pD->day = (char)iHor;
   iHor++;
   if ((pD->year & 3) == 0) {
     if (iHor > 60)
        iHor--;
     else
        if (iHor == 60) {
           pD->mon = 2;
           pD->day = 29;
           return;
        }
   }
   for (pD->mon = 0; S2Ddays[pD->mon] < (int)iHor; pD->mon++)  iHor -= S2Ddays[pD->mon];
   pD->day = (char)iHor;
   pD->mon += (char)1;
   return;
}
//----------------------------------------------------------------------------------------------------------------

/*----------------------------------------------------------------------------------------------
| GetDateTime(char *dt) | Obtiene la fecha y hora del rtc y lo retorna en un string
| Parametro: puntero a char destino de cadena
| RETURN VALUE: ninguno
-----------------------------------------------------------------------------------------------*/
void GetDateTime(char *dt){
	BYTE  		hora, min, sec, dow, dia, mes, year;

	rtc_get_date(dia, mes, year, dow);
	rtc_get_time(hora, min, sec);
	sprintf(dt, "%02d%02d%02d%02d%02d", year, mes, dia, hora, min);
}

/*----------------------------------------------------------------------------------------------
| CurrDateTime(void) | Obtiene la fecha y hora del rtc y lo convierte retorna en segundos
| Parametro: puntero a char destino de cadena
| RETURN VALUE: unsigned long (segundos)
-----------------------------------------------------------------------------------------------*/
int32 CurrDateTime(void){
	BYTE  		hora, min, sec, dow, dia, mes, year;
	TDateSt 	dt;

	rtc_get_date(dia, mes, year, dow);
	rtc_get_time(hora, min, sec);
	//
	sprintf(dtFecha, "%02d%02d%02d", year, mes, dia);
	sprintf(dtHora, "%02d:%02d", hora, min);
	//
	dt.hour = hora;
	dt.min = min;
	dt.sec = sec;
	dt.day = dia;
	dt.mon = mes;
	dt.dow = dow;
	dt.year = year + 2000;
	return Date2Sec(&dt);
}
