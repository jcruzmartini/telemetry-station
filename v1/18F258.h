//////// Standard Header file for the PIC18F258 device ////////////////
#device PIC18F258
#nolist
//////// Program memory: 16384x16  Data RAM: 1536  Stack: 31
//////// I/O: 23   Analog Pins: 5
//////// Data EEPROM: 256
//////// C Scratch area: 00   ID Location: 200000
//////// Fuses: LP,XT,HS,RC,EC,EC_IO,H4,RC_IO,PROTECT,NOPROTECT,OSCSEN
//////// Fuses: NOOSCSEN,NOBROWNOUT,BROWNOUT,WDT1,WDT2,WDT4,WDT8,WDT16,WDT32
//////// Fuses: WDT64,WDT128,WDT,NOWDT,BORV20,BORV27,BORV42,BORV45,PUT,NOPUT
//////// Fuses: CPD,NOCPD,NOSTVREN,STVREN,NODEBUG,DEBUG,NOLVP,LVP,WRT,NOWRT
//////// Fuses: NOWRTD,WRTD,WRTB,NOWRTB,CPB,NOCPB,WRTC,NOWRTC,EBTR,NOEBTR
//////// Fuses: EBTRB,NOEBTRB
//////// 
////////////////////////////////////////////////////////////////// I/O
// Discrete I/O Functions: SET_TRIS_x(), OUTPUT_x(), INPUT_x(),
//                         PORT_x_PULLUPS(), INPUT(),
//                         OUTPUT_LOW(), OUTPUT_HIGH(),
//                         OUTPUT_FLOAT(), OUTPUT_BIT()
// Constants used to identify pins in the above are:

#define PIN_A0  31744
#define PIN_A1  31745
#define PIN_A2  31746
#define PIN_A3  31747
#define PIN_A4  31748
#define PIN_A5  31749
#define PIN_A6  31750

#define PIN_B0  31752
#define PIN_B1  31753
#define PIN_B2  31754
#define PIN_B3  31755
#define PIN_B4  31756
#define PIN_B5  31757
#define PIN_B6  31758
#define PIN_B7  31759

#define PIN_C0  31760
#define PIN_C1  31761
#define PIN_C2  31762
#define PIN_C3  31763
#define PIN_C4  31764
#define PIN_C5  31765
#define PIN_C6  31766
#define PIN_C7  31767

////////////////////////////////////////////////////////////////// Useful defines
#define FALSE 0
#define TRUE 1

#define BYTE int8
#define BOOLEAN int1

#define getc getch
#define fgetc getch
#define getchar getch
#define putc putchar
#define fputc putchar
#define fgets gets
#define fputs puts

////////////////////////////////////////////////////////////////// Control
// Control Functions:  RESET_CPU(), SLEEP(), RESTART_CAUSE()
// Constants returned from RESTART_CAUSE() are:

#define WDT_TIMEOUT       7    
#define MCLR_FROM_SLEEP  11    
#define MCLR_FROM_RUN    15    
#define NORMAL_POWER_UP  12    
#define BROWNOUT_RESTART 14    
#define WDT_FROM_SLEEP   3     
#define RESET_INSTRUCTION 0    

////////////////////////////////////////////////////////////////// Timer 0
// Timer 0 (AKA RTCC)Functions: SETUP_COUNTERS() or SETUP_TIMER_0(),
//                              SET_TIMER0() or SET_RTCC(),
//                              GET_TIMER0() or GET_RTCC()
// Constants used for SETUP_TIMER_0() are:
#define RTCC_INTERNAL   0
#define RTCC_EXT_L_TO_H 32
#define RTCC_EXT_H_TO_L 48

#define RTCC_DIV_1      8
#define RTCC_DIV_2      0
#define RTCC_DIV_4      1
#define RTCC_DIV_8      2
#define RTCC_DIV_16     3
#define RTCC_DIV_32     4
#define RTCC_DIV_64     5
#define RTCC_DIV_128    6
#define RTCC_DIV_256    7

#define RTCC_OFF        0x80  

#define RTCC_8_BIT      0x40  

// Constants used for SETUP_COUNTERS() are the above
// constants for the 1st param and the following for
// the 2nd param:

////////////////////////////////////////////////////////////////// WDT
// Watch Dog Timer Functions: SETUP_WDT() or SETUP_COUNTERS() (see above)
//                            RESTART_WDT()
// WDT base is 18ms
//
#define WDT_ON      0x100   
#define WDT_OFF     0       


////////////////////////////////////////////////////////////////// Timer 1
// Timer 1 Functions: SETUP_TIMER_1, GET_TIMER1, SET_TIMER1
// Constants used for SETUP_TIMER_1() are:
//      (or (via |) together constants from each group)
#define T1_DISABLED         0
#define T1_INTERNAL         0x85
#define T1_EXTERNAL         0x87
#define T1_EXTERNAL_SYNC    0x83

#define T1_CLK_OUT          8

#define T1_DIV_BY_1         0
#define T1_DIV_BY_2         0x10
#define T1_DIV_BY_4         0x20
#define T1_DIV_BY_8         0x30

////////////////////////////////////////////////////////////////// Timer 2
// Timer 2 Functions: SETUP_TIMER_2, GET_TIMER2, SET_TIMER2
// Constants used for SETUP_TIMER_2() are:
#define T2_DISABLED         0
#define T2_DIV_BY_1         4
#define T2_DIV_BY_4         5
#define T2_DIV_BY_16        6

////////////////////////////////////////////////////////////////// Timer 3
// Timer 3 Functions: SETUP_TIMER_3, GET_TIMER3, SET_TIMER3
// Constants used for SETUP_TIMER_3() are:
//      (or (via |) together constants from each group)
#define T3_DISABLED         0
#define T3_INTERNAL         0x85
#define T3_EXTERNAL         0x87
#define T3_EXTERNAL_SYNC    0x83

#define T3_DIV_BY_1         0
#define T3_DIV_BY_2         0x10
#define T3_DIV_BY_4         0x20
#define T3_DIV_BY_8         0x30

////////////////////////////////////////////////////////////////// CCP
// CCP Functions: SETUP_CCPx, SET_PWMx_DUTY
// CCP Variables: CCP_x, CCP_x_LOW, CCP_x_HIGH
// Constants used for SETUP_CCPx() are:
#define CCP_OFF                         0
#define CCP_CAPTURE_FE                  4
#define CCP_CAPTURE_RE                  5
#define CCP_CAPTURE_DIV_4               6
#define CCP_CAPTURE_DIV_16              7
#define CCP_COMPARE_SET_ON_MATCH        8
#define CCP_COMPARE_CLR_ON_MATCH        9
#define CCP_COMPARE_INT                 0xA
#define CCP_COMPARE_INT_AND_TOGGLE      0x2       
#define CCP_COMPARE_RESET_TIMER         0xB
#define CCP_PWM                         0xC
#define CCP_PWM_PLUS_1                  0x1c
#define CCP_PWM_PLUS_2                  0x2c
#define CCP_PWM_PLUS_3                  0x3c
#define CCP_USE_TIMER3                  0x100       
long CCP_1;
#byte   CCP_1    =                      0xfbe       
#byte   CCP_1_LOW=                      0xfbe       
#byte   CCP_1_HIGH=                     0xfbf       
////////////////////////////////////////////////////////////////// SPI
// SPI Functions: SETUP_SPI, SPI_WRITE, SPI_READ, SPI_DATA_IN
// Constants used in SETUP_SPI() are:
#define SPI_MASTER       0x20
#define SPI_SLAVE        0x24
#define SPI_L_TO_H       0
#define SPI_H_TO_L       0x10
#define SPI_CLK_DIV_4    0
#define SPI_CLK_DIV_16   1
#define SPI_CLK_DIV_64   2
#define SPI_CLK_T2       3
#define SPI_SS_DISABLED  1

#define SPI_SAMPLE_AT_END 0x8000
#define SPI_XMIT_L_TO_H  0x4000

////////////////////////////////////////////////////////////////// UART
// Constants used in setup_uart() are:
// FALSE - Turn UART off
// TRUE  - Turn UART on
#define UART_ADDRESS           2
#define UART_DATA              4
////////////////////////////////////////////////////////////////// VREF
// Constants used in setup_low_volt_detect() are:
//
#define LVD_LVDIN   0x1F
#define LVD_45 0x1E
#define LVD_42 0x1D
#define LVD_40 0x1C
#define LVD_38 0x1B
#define LVD_36 0x1A
#define LVD_35 0x19
#define LVD_33 0x18
#define LVD_30 0x17
#define LVD_28 0x16
#define LVD_27 0x15
#define LVD_25 0x14
#define LVD_23 0x13    
#define LVD_21 0x12    
#define LVD_19 0x11    


////////////////////////////////////////////////////////////////// INTERNAL RC
// Constants used in setup_oscillator() are:
#define OSC_TIMER1  1
#define OSC_NORMAL  0


////////////////////////////////////////////////////////////////// ADC
// ADC Functions: SETUP_ADC(), SETUP_ADC_PORTS() (aka SETUP_PORT_A),
//                SET_ADC_CHANNEL(), READ_ADC()
// Constants used for SETUP_ADC() are:
#define ADC_OFF                 0              // ADC Off
#define ADC_CLOCK_DIV_2   0x10000
#define ADC_CLOCK_DIV_4    0x4000
#define ADC_CLOCK_DIV_8    0x0040
#define ADC_CLOCK_DIV_16   0x4040
#define ADC_CLOCK_DIV_32   0x0080
#define ADC_CLOCK_DIV_64   0x4080
#define ADC_CLOCK_INTERNAL 0x00c0              // Internal 2-6us

// Constants used in SETUP_ADC_PORTS() are:
#define NO_ANALOGS                           7    // None
#define ALL_ANALOG                           0    // A0 A1 A2 A3 A4         
#define AN0_AN1_AN2_AN4_VSS_VREF             3    // A0 A1 A2 A4 VRefh=A3              
#define AN0_AN1_AN3                          4    // A0 A1 A3
#define AN0_AN1_VSS_VREF                     5    // A0 A1 VRefh=A3
#define AN0_AN1_AN4_VREF_VREF             0x08    // A0 A1 A4 VRefh=A3 VRefl=A2              
#define AN0_AN1_VREF_VREF                 0x0D    // A0 A1 VRefh=A3 VRefl=A2
#define AN0                               0x0E    // A0
#define AN0_VREF_VREF                     0x0F    // A0 VRefh=A3 VRefl=A2
#define ANALOG_RA3_REF         0x1         //!old only provided for compatibility
#define RA0_RA1_RA3_ANALOG     0x4         //!old only provided for compatibility
#define RA0_RA1_ANALOG_RA3_REF 0x5         //!old only provided for compatibility
#define ANALOG_RA3_RA2_REF              0x8   //!old only provided for compatibility
#define RA0_RA1_ANALOG_RA3_RA2_REF      0xD   //!old only provided for compatibility
#define RA0_ANALOG                      0xE   //!old only provided for compatibility
#define RA0_ANALOG_RA3_RA2_REF          0xF   //!old only provided for compatibility


// Constants used in READ_ADC() are:
#define ADC_START_AND_READ     7   // This is the default if nothing is specified
#define ADC_START_ONLY         1
#define ADC_READ_ONLY          6





////////////////////////////////////////////////////////////////// INT
// Interrupt Functions: ENABLE_INTERRUPTS(), DISABLE_INTERRUPTS(),
//                      CLEAR_INTERRUPT(), INTERRUPT_ACTIVE(),
//                      EXT_INT_EDGE()
//
// Constants used in EXT_INT_EDGE() are:
#define L_TO_H              0x40
#define H_TO_L                 0
// Constants used in ENABLE/DISABLE_INTERRUPTS() are:
#define GLOBAL                    0xF2C0
#define INT_RTCC                  0xF220
#define INT_TIMER0                0xF220
#define INT_TIMER1                0x9D01
#define INT_TIMER2                0x9D02
#define INT_TIMER3                0xA002
#define INT_EXT                   0xF210
#define INT_EXT1                  0xF008
#define INT_EXT2                  0xF010
#define INT_RB                    0xFFF208
#define INT_AD                    0x9D40
#define INT_RDA                   0x9D20
#define INT_TBE                   0x9D10
#define INT_SSP                   0x9D08
#define INT_CCP1                  0x9D04
#define INT_BUSCOL                0xA008
#define INT_LOWVOLT               0xA004
#define INT_CANIRX                0xA380
#define INT_CANWAKE               0xA340
#define INT_CANERR                0xA320
#define INT_EEPROM                0xA010
#define INT_CANTX2                0xA310
#define INT_CANTX1                0xA308
#define INT_CANTX0                0xA304
#define INT_CANRX1                0xA302
#define INT_CANRX0                0xA301

#list
