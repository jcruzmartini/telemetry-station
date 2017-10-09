#define Echo_Off   0   //
#define Dial_Check 1   //?
#define Text_On    2   //?
#define Dial_In    3   //?
#define Wait_Reg   4   //?
#define Read_Nom   5   //?
#define Wait       7   //?
#define Wait1      8   //?
#define DialWait   9   //?
#define SmsWait    10  //?
#define Dial_Wait_Discon 11
#define Dial_Ans   12
#define Dial_Ans_Wait  13
#define SmsSent    14
#define SmsCom     15
#define SmsDel     16                                                                                                                                                                                                                                      
#define Ath        20
#define Aux        21
#define Mic_Lev    22
//                                                                                                                                                                            1        1             1            1       1     1    1         1           1           1         1             1
//                                               1     1            2             3             4            5            6            7           8        9         9       0        0             2            3       3     3    3         4           5           6         6             7
//                        0     3        8       2     5            4             5             6            6            6            6           5        1         8       3        9             0            0       3     6    8         5           4           3         9             9
const char *ALLSTR     = "AT\000ATE0\000\ATA\000\OK\000AT+CREG?\000\+CREG: 0,1\000+CREG: 0,5\000AT+COLP=1\000AT+CMGF=1\000AT+CLIP=1\000AT+CMGS=\000CPBF:\000+CLIP:\000BUSY\000ERROR\000NO CARRIER\000NO ANSWER\000,\"\000\",\000>\000+CMTI:\000AT+CMGR=\000AT+CMGD=\000ATD>\"\000AT+CPBF=\"\000ATH\000";

#define _AT          0  	//= "AT";
#define _ECHO_OFF    3  	//= "ATE0";
#define _ATA         8  	//= "ATA";
#define _OK          12 	//= "OK";
#define _REG         15 	//= "AT+CREG?";   //
#define _REG_OK1     24 	//= "+CREG: 0,1"; //?
#define _REG_OK5     35 	//= "+CREG: 0,5"; //?
#define _COLP        46 	//= "AT+COLP=1";  //?
#define _TEXT_MODE   56 	//= "AT+CMGF=1";  //?
#define _DIAL_IN     66 	//= "AT+CLIP=1";  //?
#define _SENT_SMS    76 	//= "AT+CMGS=";
#define _READ_NOM_OK 85 	//= "CPBF:";
#define _RING        91 	//= "+CLIP:";
#define _BUSY        98 	//= "BUSY";  		//
#define _ERROR       103 	//= "ERROR";  	//?
#define _NO_CAR      109        //= "NO CARRIER";
#define _NO_ANS      120        //= "NO ANSWER";
#define _ZP_KV       130        //= ",\"";
#define _KV_ZP       133        //= "\",";
#define _SMS_TEXT    136        //  = ">";
#define _RES_SMS     138        //= "+CMTI:";
#define _CMGR        145        //= "AT+CMGR=";
#define _DELSMS      154        //= "AT+CMGD=";
#define _ATD         163        //= "ATD>\"";
#define _READ_PHONE_NOM 169 //= "AT+CPBF=\"";
#define _ATH         179        //= "ATH";



//const char *MIC1   = "AT+CMIC=0,0";
//const char *MIC2   = "AT+CSMP=17,167,0,0";
const char *MIC2   = "AT+CMIC=0,0";
const char *CHF    = "AT+CHF=0,1";


unsigned char Trans_Str(const char *s)
{
unsigned char i;

for (i=0;i<32;i++)
        {TRANSMIT[i];}

for (i=0;i<78;i++)
        {RESIV[i]=0;}

i=0;

while (*s)
        {
        TRANSMIT[i]=(*s);
        i++;
        s++;
        }
return i;
}      


void Trans_Str_CR(const char *s)
{
TRANSMIT[Trans_Str(s)]=0x0D;
}


unsigned char Read_Phone(unsigned char i)
{
unsigned char j=0;
unsigned char bank2 *s;
unsigned char bank2 *e;

s=strstr(RESIV,&ALLSTR[_ZP_KV]);
e=strstr(RESIV,&ALLSTR[_KV_ZP]);
s+=2;

if (s<e)
        {ALARM_MASK_DIAL|=(0b000010101<<i);}


while ((j<15) && (s<e))
        {
        while (!(*s==EEPROM_READ(j+(i*16))))
                {
                EEPROM_WRITE(j+(i*16),*s);
                while (WR)
                        {}
                }
        s++;
        j++;
        }


while (!(0==EEPROM_READ(j+(i*16))))
        {
        EEPROM_WRITE(j+(i*16),0);
        while (WR)
                {}
        }
return 1;
}


void Read_Sms_Nom(void)
{
unsigned char bank2 *e;
unsigned char j=0;

e=strstr(RESIV,&ALLSTR[_KV_ZP]);
e+=2;

RES_SMS_NOM[0]=(*e);
RES_SMS_NOM[1]=0;
RES_SMS_NOM[2]=0;
e++;
if ((*e)|=0x0D)
        {RES_SMS_NOM[1]=*e;}
}



void Go_Ath(void)
{
if (strstr(RESIV,&ALLSTR[_BUSY]))
        {REGIM=Ath;}
if (strstr(RESIV,&ALLSTR[_ERROR]))
        {REGIM=Ath;}
if (strstr(RESIV,&ALLSTR[_NO_CAR]))
        {REGIM=Ath;}
if (strstr(RESIV,&ALLSTR[_NO_ANS]))
        {REGIM=Ath;}
}



void Peredacha(void)
{
unsigned char i=0;
unsigned char j=0;
unsigned char k=0;
unsigned char e=0;

Kol_Resiv=0;
Kol_Transmit=0;
KOL_SOFT_TRANSMIT=0;

WAIT_ANSWER=100;

switch (REGIM)
        {                      
        case Echo_Off:
                Trans_Str_CR(&ALLSTR[_ECHO_OFF]);
                En_Transmit=1;
                break;

        case Dial_Check:
                Trans_Str_CR(&ALLSTR[_COLP]);
                En_Transmit=1;
                break;

        case Text_On:
                Trans_Str_CR(&ALLSTR[_TEXT_MODE]);
                En_Transmit=1;
                break;

        case Aux:
                Trans_Str_CR(CHF);
                En_Transmit=1;
                break;

        case Mic_Lev:
                Trans_Str_CR(MIC2);
                En_Transmit=1;
                break;

        case Dial_In:
                Trans_Str_CR(&ALLSTR[_DIAL_IN]);
                En_Transmit=1;
                break;

        case Wait_Reg:
                Trans_Str_CR(&ALLSTR[_REG]);
                En_Transmit=1;
                break;

        case Read_Nom:
                WAIT_ANSWER=250;
                i=Trans_Str(&ALLSTR[_READ_PHONE_NOM]);
                while (PHONE_NAME[j+(PHONE_NOM*8)])
                        {
                        TRANSMIT[i++]=PHONE_NAME[j+(PHONE_NOM*8)];
                        j++;
                        }
                TRANSMIT[i++]='"';
                TRANSMIT[i]=0x0D;
                En_Transmit=1;
                break;

        case Wait:

        case Wait1:
                CUR_TASK=Alarm_Com();
                if (CUR_TASK==0)
                        {
                        WAIT_ANSWER=50;
                        Check_Reg=1;
                        Trans_Str_CR(&ALLSTR[_REG]);
                        En_Transmit=1;
                        }
                else
                        {
                        if (CUR_TASK&0b00001111)
                                {
                                WAIT_ANSWER=100;
                                i=Trans_Str(&ALLSTR[_ATD]);
                                j=0;
                                while (PHONE_NAME[j+(((CUR_TASK&0b00001111)-1)*8)])
                                        {
                                        TRANSMIT[i++]=PHONE_NAME[j+(((CUR_TASK&0b00001111)-1)*8)];
                                        j++;
                                        }
                                TRANSMIT[i++]='"';
                                TRANSMIT[i++]=';';
                                TRANSMIT[i]=0x0D;
                                REGIM=DialWait;
                                En_Transmit=1;
                                }
                        else
                                {
                                if (CUR_TASK&0b11110000)
                                        {
                                        i=Trans_Str(&ALLSTR[_SENT_SMS]);
                                        j=0;
                                        TRANSMIT[i++]='"';
                                        while (EEPROM_READ(((((CUR_TASK&0b11110000)>>4)-1)*16)+j))
                                                {
                                                TRANSMIT[i]=EEPROM_READ(((((CUR_TASK&0b11110000)>>4)-1)*16)+j);
                                                i++;
                                                j++;
                                                }
                                        TRANSMIT[i++]='"';
                                        TRANSMIT[i]=0x0D;
                                        REGIM=SmsWait;
                                        En_Transmit=1;
                                        }
                                }
                        }
                break;

        case DialWait:
                break;

        case SmsWait:
                break;

        case Dial_Wait_Discon:
                Dial_Wait_Com();
                break;

        case Dial_Ans:
                Trans_Str_CR(&ALLSTR[_ATA]);
                En_Transmit=1;
                break;

        case Dial_Ans_Wait:  
                Dial_Ans_Com();
                break;

        case Ath:
                Trans_Str_CR(&ALLSTR[_ATH]);
                En_Transmit=1;
                break;

        case SmsSent:
                Trans_Str_CR(&ALLSTR[_AT]);
                k=0;
                for (j=0;j<=7;j++)
                        {
                        if (CUR_OP&(1<<j))
                                {
                                e=0;
                                while (SMS_ALARM[j*8+e])
                                        {
                                        TRANSMIT[k++]=SMS_ALARM[j*8+e];
                                        e++;
                                        }
                                TRANSMIT[k++]=' ';
                                }
                        }
                TRANSMIT[k++]=0x1A;
                En_Transmit=1;
                break;

        case SmsCom:
                i=Trans_Str(&ALLSTR[_CMGR]);
                Pre_Read_Sms=1;
                TRANSMIT[i++]=RES_SMS_NOM[0];
                if (RES_SMS_NOM[1])
                        {TRANSMIT[i++]=RES_SMS_NOM[1];}
                TRANSMIT[i]=0x0D;
                En_Transmit=1;
                break;

        case SmsDel:
                i=Trans_Str(&ALLSTR[_DELSMS]);
                TRANSMIT[i++]=RES_SMS_NOM[0];
                if (RES_SMS_NOM[1])
                        {TRANSMIT[i++]=RES_SMS_NOM[1];}
                TRANSMIT[i]=0x0D;
                En_Transmit=1;
                break;

        }
}




void Priem(void)
{

WAIT_ANSWER=0;

if (Sms_In_En)
        {
        if (strstr(RESIV,&ALLSTR[_RES_SMS]))
                {Resiv_Sms();}
        }

switch (REGIM)
        {                          

        case Echo_Off:
                if (strstr(RESIV,&ALLSTR[_OK]))
                        {REGIM=Dial_Check;}
                break;

        case Dial_Check:
                if (strstr(RESIV,&ALLSTR[_OK]))
                        {REGIM=Text_On;}
                break;

        case Text_On:
                if (strstr(RESIV,&ALLSTR[_OK]))
                        {REGIM=Aux;}
                break;


        case Aux:
                if (strstr(RESIV,&ALLSTR[_OK]))
                        {REGIM=Mic_Lev;}
                break;


        case Mic_Lev:
                if (strstr(RESIV,&ALLSTR[_OK]))
                        {REGIM=Dial_In;}
                break;


        case Dial_In:
                if (strstr(RESIV,&ALLSTR[_OK]))
                        {REGIM=Wait_Reg;}
                break;


        case Wait_Reg:
                if ((strstr(RESIV,&ALLSTR[_REG_OK1]))||(strstr(RESIV,&ALLSTR[_REG_OK5])))
                        {
                        if (Max_Phone_Nom>0)
                                {REGIM=Read_Nom;}
                        else
                                {REGIM=Wait;}
                        PHONE_NOM=0;
                        }
                break;

        case Read_Nom:
                WAIT_RESTART=Max_Wait_Answer;

                if (strstr(RESIV,&ALLSTR[_READ_NOM_OK]))
                        {Read_Phone(PHONE_NOM);}

                if (strstr(RESIV,&ALLSTR[_OK]))
                        {
                        PHONE_NOM++;
                        if (PHONE_NOM>Max_Phone_Nom)
                                {
                                REGIM=Wait;
                                PHONE_NOM=0;
                                }
                        }
                break;

        case Wait:
        case Wait1:
                if ((strstr(RESIV,&ALLSTR[_REG_OK1]) || strstr(RESIV,&ALLSTR[_REG_OK5])))
                        {
                        if (REGIM==Wait)
                                {REGIM=Wait1;}
                        else
                                {REGIM=Wait;}
                        }
                else
                        {
                        if (Check_Reg)
                                {
                                PwrKey=1;
                                DelayMs(250);
                                DelayMs(250);
                                DelayMs(250);
                                PwrKey=0;
                                DelayMs(250);
                                DelayMs(250);
                                DelayMs(250);
                                DelayMs(250);
                                DelayMs(250);
                                DelayMs(250);
                                REGIM=Echo_Off;
                                }
                        }

                if (strstr(RESIV,&ALLSTR[_RING]))
                        {REGIM=Ath;}

                if (RES_SMS_NOM[0]>0)
                        {REGIM=SmsCom;}

                Check_Reg=0;

                break;

        case DialWait:
                CCPR1L=0;
                if (strstr(RESIV,&ALLSTR[_OK]))
                        {Dial_Ok();}
                Go_Ath();
                break;

        case SmsWait:
                REGIM=Wait;
                if (strstr(RESIV,&ALLSTR[_SMS_TEXT]))
                        {REGIM=SmsSent;}
                break;


        case Dial_Wait_Discon:
                Go_Ath();
                break;

        case Dial_Ans:
                REGIM=Dial_Ans_Wait;
                Go_Ath();
                break;

        case Dial_Ans_Wait:
                Go_Ath();
                break;


        case Ath:
                if (strstr(RESIV,&ALLSTR[_OK]))
                        {REGIM=Wait;}
                break;

        case SmsSent:
                if (strstr(RESIV,&ALLSTR[_OK]))
                        {Sms_Sent_Ok();}
                REGIM=Wait;
                break;


        case SmsCom:
                REGIM=SmsDel;
                Read_And_Com_Sms();
                break;


        case SmsDel:
                RES_SMS_NOM[0]=0;
                REGIM=Wait;
                break;
        }
}
