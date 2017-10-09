//----------------------------------------------------------------------------- 
//  This files handles the communication between a microcontroller
//  and an I2C LCD driven by PCF8574T
//  PCF8574T     LCD 
//  ========     ====== 
//     P0        RS            0b00000001  //P0 - PCF8574T Pin connected to RS 
//     P1        RW            0b00000010  //P1 - PCF8574T Pin connected to RW 
//     P2        Enable        0b00000100  //P2 - PCF8574T Pin connected to EN 
//     P3        Led Backlight 0b00001000  //P3 - PCF8574T Pin connected to BACKLIGHT LED 
//     P4        D4 
//     P5        D5 
//     P6        D6 
//     P7        D7 
// 
//----------------------------------------------------------------------------- 


#ifndef LCD_ADDR
    #define LCD_ADDR   0x4E //I2C slave address for LCD module 
#endif

#define PIN_RS         0
#define PIN_RW         1
#define PIN_EN         2
#define PIN_BACKLIGHT  3


#define lcd_line_one   0x80   // LCD RAM address for line 1 
#define lcd_line_two   0xC0   // LCD RAM address for line 2 
#define lcd_line_three 0x94   // LCD RAM address for line 3 
#define lcd_line_four  0xD4   // LCD RAM address for line 4 


void lcdWrite(unsigned int8);
byte address; 
int1 lcd_backlight = 0;

void i2cLCD_writeLatch(unsigned int8 data) {    
  i2c_start(); 
  i2c_write(LCD_ADDR); //the slave addresse W mode
  i2c_write(data); 
  i2c_stop(); 
} 

unsigned int8 i2cLCD_readLatch(void) {    
  unsigned int8 data;
  i2c_start(); 
  i2c_write(LCD_ADDR + 1); //the slave addresse R mode
  data = i2c_read(0); 
  i2c_stop(); 
  return data;
} 

void lcd_send_byte(unsigned int8 data) { 
  bit_set(data,PIN_EN);
  if (lcd_backlight){
    bit_set(data, PIN_BACKLIGHT);
  }
  i2cLCD_writeLatch(data); 
  bit_clear(data,PIN_EN);
  i2cLCD_writeLatch(data); 
} 
    
void i2cLCD_clear() { 
  lcd_send_byte(0x00); 
  lcd_send_byte(0x10); 
  delay_ms(2); 
} 

void i2cLCD_message(char *text) {
  while (*text) {
    lcdWrite(*text++);
  }
  return;
}

void i2cLCD_init() { 
  delay_ms(200); //LCD power up delay 
        
  //Request works on the command by set the RS = 0 R/W = 0 write 
  lcd_send_byte(0x00); 
  lcd_send_byte(0x10); 
  lcd_send_byte(0x00); 
  lcd_send_byte(0x00); 
  lcd_send_byte(0x10); 
  //First state in 8 bit mode 
  lcd_send_byte(0x30); 
  lcd_send_byte(0x30); 
  //Then set to 4-bit mode 
  lcd_send_byte(0x30); 
  lcd_send_byte(0x20); 
  //mode 4 bits, 2 lines, characters 5 x 7 (28 h) 
  lcd_send_byte(0x20); 
  lcd_send_byte(0x80); 
  //no need cursor on (0Ch) 
  lcd_send_byte(0x00); 
  lcd_send_byte(0xC0); 
  //the cursor moves to the left (06 h) 
  lcd_send_byte(0x00); 
  lcd_send_byte(0x60); 
  //clears the display 
  i2cLCD_clear(); 
} 

void i2cLCD_cursorPosition(byte y, byte x) {      
  static unsigned int8 data;  
  switch(y) { 
     case 0:  address = lcd_line_one;     break; 
     case 1:  address = lcd_line_two;     break; 
     case 2:  address = lcd_line_three;   break; 
     case 3:  address = lcd_line_four;    break; 
     default: address = lcd_line_one;     break;  
  } 
  
  address+= x-1; 
  data = address&0xF0; 
  lcd_send_byte(data); 
  data = address&0x0F; 
  data = data<<4; 
  lcd_send_byte(data);
} 

void lcdWrite(unsigned int8 in_data) { 
  unsigned int8 data;  
  data = in_data&0xF0; 
  bit_set(data,PIN_RS);
  lcd_send_byte(data); 
  data = in_data&0x0F; 
  data = data<<4; 
  bit_set(data,PIN_RS);
  lcd_send_byte(data); 
}

void i2cLCD_backlight(int1 backlight) {
  unsigned int8 current = i2cLCD_readLatch();
  if (backlight) {
     bit_set(current, PIN_BACKLIGHT);
  } else {
	 bit_clear(current, PIN_BACKLIGHT);
  }
  lcd_backlight = backlight;
  i2cLCD_writeLatch(current);
}
