/* This files handles the communication between a microcontroller
   and an I2C LCD driven by MCP23008
   Tate Peñaranda - tate@xenir.com.ar
*/

#ifndef I2C_BUS_SCL
  #define I2C_BUS_SCL    PIN_B4
  #define I2C_BUS_SDA    PIN_B5
  #define I2C_BUS_STREAM i2cPort
#endif

#ifndef LCD_ADDR
  #define  LCD_ADDR       0x40
#endif

#define MCP23008_IODIR    0x00
#define MCP23008_IPOL     0x01
#define MCP23008_GPINTEN  0x02
#define MCP23008_DEFVAL   0x03
#define MCP23008_INTCON   0x04
#define MCP23008_IOCON    0x05
#define MCP23008_GPPU     0x06
#define MCP23008_INTF     0x07
#define MCP23008_INTCAP   0x08
#define MCP23008_GPIO     0x09
#define MCP23008_OLAT     0x0A

#define LCD_RS            7
#define LCD_E             6
#define LCD_BIT4          5
#define LCD_BIT5          4
#define LCD_BIT6          3
#define LCD_BIT7          2
#define LCD_BACKLIGHT     1
#define LCD_NC            0

void ePinHighMs(unsigned int8);
void lcdWrite(unsigned int1, unsigned int8);
void rSPinLow(void);
void rSPinHigh(void);
void i2cLCD_message(char);
void i2cLCD_clear(void);
void i2cLCD_writeLatch(unsigned int8);
void i2cLCD_homeLine(unsigned int1, unsigned int1);
void i2cLCD_cursorDirection(unsigned int1, unsigned int8);
void i2cLCD_sendASCII(unsigned int8);
unsigned int8 i2cLCD_readLatch(void);

void i2cLCD_init(void) {
  unsigned int8 outputData;
  outputData = 0;

  i2c_start(i2cPort);
  i2c_write(i2cPort, LCD_ADDR);
  i2c_write(i2cPort, MCP23008_IODIR);
  i2c_write(i2cPort, outputData);
  i2c_stop(i2cPort);

  i2cLCD_writeLatch(outputData);
  delay_ms(20);

  bit_set(outputData, LCD_BIT4);
  bit_set(outputData, LCD_BIT5);

  i2cLCD_writeLatch(outputData);
  ePinHighMs(2);
  delay_ms(3);

  i2cLCD_writeLatch(outputData);
  ePinHighMs(2);
  delay_ms(3);

  i2cLCD_writeLatch(outputData);
  ePinHighMs(2);
  delay_ms(3);

  outputData = 0;
  bit_set(outputData, LCD_BIT5);

  i2cLCD_writeLatch(outputData);
  ePinHighMs(2); // 4bit mode switch
  delay_ms(3);

  //Additionals init instructions
  lcdWrite(0, 0b00000110);
  delay_ms(1);
  lcdWrite(0, 0b00001100);
  delay_ms(1);
  lcdWrite(0, 0b00010100);
  delay_ms(1);
  lcdWrite(0, 0b00101000);
  delay_ms(1);
  lcdWrite(0, 0b00000010);
  delay_ms(1);
  lcdWrite(0, 0b00000001);
  delay_ms(5);
}

void i2cLCD_clear(void) {
  lcdWrite(0, 0b00000010);
  delay_ms(1);
  lcdWrite(0, 0b00000001);
  delay_ms(1);
}

void i2cLCD_cursorDirection(unsigned int1 direction) {
  unsigned int8 command = direction ? 0b00000110 : 0b00000100;
  lcdWrite(0, command);
  delay_us(40);
}

void i2cLCD_homeLine(unsigned int1 row, unsigned int1 clear) {
  unsigned int8 command = row ? 0b11000000 : 0b10000000;
  lcdWrite(0, command);
  if (clear) {
    int i;
    for (i = 0; i < 16; ++i) {
      i2cLCD_sendASCII(" ");
      lcdWrite(0, command);
    }
  }
  delay_ms(1);
}

void i2cLCD_cursorPosition(unsigned int1 row, unsigned int8 column) {
  unsigned int8 command = row ? (0b11000000 | column) : (0b10000000 | column);
  lcdWrite(0, command);
  delay_ms(1);
}

void lcdWrite(unsigned int1 rs, unsigned int8 data) {
  unsigned int8 highNibble = (data >> 4) & 0b00001111;
  unsigned int8 lowNibble = data & 0b00001111;
  unsigned int8 nibbles[2], outputData, i, currentLatch;

  currentLatch = i2cLCD_readLatch();

  nibbles[0] = highNibble;
  nibbles[1] = lowNibble;

  for (i = 0; i < 2; ++i) {
    outputData = 0;

    if (bit_test(nibbles[i], 0)) bit_set(outputData, 5);
    if (bit_test(nibbles[i], 1)) bit_set(outputData, 4);
    if (bit_test(nibbles[i], 2)) bit_set(outputData, 3);
    if (bit_test(nibbles[i], 3)) bit_set(outputData, 2);
    if (bit_test(currentLatch, LCD_BACKLIGHT)) bit_set(outputData, LCD_BACKLIGHT);
    if (rs) bit_set(outputData, LCD_RS);

    i2cLCD_writeLatch(outputData);

    ePinHighMs(1);
  }

}

void i2cLCD_backlight(unsigned int1 backlight) {
  unsigned int8 currentLatch;

  currentLatch = i2cLCD_readLatch();

  if ( bit_test(currentLatch, LCD_BACKLIGHT) != backlight ) {
    (backlight) ? bit_set(currentLatch, LCD_BACKLIGHT) : bit_clear(currentLatch, LCD_BACKLIGHT);
    i2cLCD_writeLatch(currentLatch);
  }

  return;
}

void ePinHighMs(unsigned int8 ms) {
  unsigned int8 currentLatch;

  currentLatch = i2cLCD_readLatch();

  if (bit_test(currentLatch, LCD_E)) return;

  bit_set(currentLatch, LCD_E);
  i2cLCD_writeLatch(currentLatch);

  delay_ms(ms);

  bit_clear(currentLatch, LCD_E);
  i2cLCD_writeLatch(currentLatch);
  return;
}

void rSPinHigh(void) {
  unsigned int8 currentLatch;

  currentLatch = i2cLCD_readLatch();

  if (bit_test(currentLatch, LCD_RS)) return;

  bit_set(currentLatch, LCD_E);
  i2cLCD_writeLatch(currentLatch);
  return;
}

void rSPinLow(void) {
  unsigned int8 currentLatch;

  currentLatch = i2cLCD_readLatch();

  if (!bit_test(currentLatch, LCD_RS)) return;

  bit_clear(currentLatch, LCD_E);
  i2cLCD_writeLatch(currentLatch);
  return;
}

void i2cLCD_message(char *text) {
  while (*text) {
    lcdWrite(1, *text++);
  }
  return;
}

void i2cLCD_sendASCII(unsigned int8 ascii) {
  if (ascii < 0x20 || ascii > 0x7A) {
    lcdWrite(1, '?');
  } else {
    lcdWrite(1, ascii);
  }
  return;
}

void i2cLCD_writeLatch(unsigned int8 outputData) {
  i2c_start(i2cPort);
  i2c_write(i2cPort, LCD_ADDR);
  i2c_write(i2cPort, MCP23008_OLAT);
  i2c_write(i2cPort, outputData);
  i2c_stop(i2cPort);
  return;
}

unsigned int8 i2cLCD_readLatch(void) {
  unsigned int8 currentLatch;

  i2c_start(i2cPort);
  i2c_write(i2cPort, LCD_ADDR);
  i2c_write(i2cPort, MCP23008_OLAT);
  i2c_start(i2cPort);
  i2c_write(i2cPort, LCD_ADDR + 1);
  currentLatch = i2c_read(i2cPort, 0);
  i2c_stop(i2cPort);

  return currentLatch;
}