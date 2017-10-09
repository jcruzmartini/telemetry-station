#ifndef TEMP_GND_PIN
   #define TEMP_GND_PIN  PIN_B0 // One Wire Bus pin assignment 
#endif

#define DS1820_CONN_ERROR 1111.0 // error sign 

// One Wire bus functions - from Dallas publication AN162 
// "Interfacing DS18x20/DS1822 1-wire Temperature Sensor in a Microcontroller 
// Environment". Delays calculated from values given for 8051. 
// Changed variable name ROM[] to RomBytes[] because ROM is a reserved word 
// in version 4 of the CCS compiler. 

// Global variables 
int8 RomBytes[8];      
int8 lastDiscrep = 0; 
short doneFlag = 0; 
int8 FoundROM[9][8];    // Table of found ROM codes, 8 bytes for each 
int8 numROMs; 
int8 dowcrc;            // crc is accumulated in this variable 

// crc lookup table 
int8 const dscrc_table[] = { 
   0,94,188,226,97,63,221,131,194,156,126,32,163,253,31,65, 
   157,195,33,127,252,162,64,30,95,1,227,189,62,96,130,220, 
   35,125,159,193,66,28,254,160,225,191,93,3,128,222,60,98, 
   190,224,2,92,223,129,99,61,124,34,192,158,29,67,161,255, 
   70,24,250,164,39,121,155,197,132,218,56,102,229,187,89,7, 
   219,133,103,57,186,228,6,88,25,71,165,251,120,38,196,154, 
   101,59,217,135,4,90,184,230,167,249,27,69,198,152,122,36, 
   248,166,68,26,153,199,37,123,58,100,134,216,91,5,231,185, 
   140,210,48,110,237,179,81,15,78,16,242,172,47,113,147,205, 
   17,79,173,243,112,46,204,146,211,141,111,49,178,236,14,80, 
   175,241,19,77,206,144,114,44,109,51,209,143,12,82,176,238, 
   50,108,142,208,83,13,239,177,240,174,76,18,145,207,45,115, 
   202,148,118,40,171,245,23,73,8,86,180,234,105,55,213,139, 
   87,9,235,181,54,104,138,212,149,203,41,119,244,170,72,22, 
   233,183,85,11,136,214,52,106,43,117,151,201,74,20,246,168, 
   116,42,200,150,21,75,169,247,182,232,10,84,215,137,107,53 
}; 

// Returns 0 for one wire device presence, 1 for none 
int8 ow_reset(void) 
{ 
   int8 presence; 

   output_low(TEMP_GND_PIN); 
   delay_us(488);          // Min. 480uS 
   output_float(TEMP_GND_PIN); 
   delay_us(72);           // Takes 15 to 60uS for devices to respond 
   presence = input(TEMP_GND_PIN); 
   delay_us(424);          // Wait for end of timeslot 
   return(presence); 
} 
//****************************************************************************** 
// Read bit on one wire bus 
int8 read_bit(void) 
{ 
   output_low(TEMP_GND_PIN); 
   delay_us(1);         // Added, 1uS min. Code relied on 8051 being slow. 
   output_float(TEMP_GND_PIN); 
   delay_us(12);        // Read within 15uS from start of time slot 
   return(input(TEMP_GND_PIN));    
}                        
//****************************************************************************** 
void write_bit(int8 bitval) 
{ 
   output_low(TEMP_GND_PIN); 

   if(bitval == 1) { 
      delay_us(1);      // 1uS min. Code relied on 8051 being slow. 
      output_float(TEMP_GND_PIN); 
   } 
   delay_us(105);       // Wait for end of timeslot 
   output_float(TEMP_GND_PIN); 
} 
//****************************************************************************** 
int8 read_byte(void) 
{ 
   int8 i; 
   int8 val = 0; 

   for(i=0;i<8;i++) 
   { 
      if(read_bit()) val |= (0x01 << i); 
      delay_us(120);  // To finish time slot 
   } 

   return val; 
} 
//****************************************************************************** 
void write_byte(int8 val) 
{ 
   int8 i; 
   int8 temp; 

   for (i=0;i<8;i++) 
   { 
      temp = val >> i; 
      temp &= 0x01; 
      write_bit(temp); 
   } 

   delay_us(105); 
} 
//****************************************************************************** 
// One wire crc 
int8 ow_crc(int8 x) 
{ 
   dowcrc = dscrc_table[dowcrc^x]; 
   return dowcrc; 
} 
//****************************************************************************** 
// Searches for the next device on the one wire bus. If there are no more 
// devices on the bus then false is returned. 
int8 Next(void) 
{ 
   int8 m = 1;             // ROM Bit index 
   int8 n = 0;             // ROM Byte index 
   int8 k = 1;             // Bit mask 
   int8 x = 0; 
   int8 discrepMarker = 0; 
   int8 g;                 // Output bit 
   int8 nxt;               // Return value 
   short flag; 


   nxt = FALSE;            // Reset next flag to false 

   dowcrc = 0;             // Reset the dowcrc 

   flag = ow_reset(); 

   if (flag||doneFlag)     // If no parts return false 
   { 
      lastDiscrep = 0;     // Reset the search 
      return FALSE; 
   } 

   write_byte(0xF0);       // Send SearchROM command 

   do 
   { 
      x = 0; 
      if (read_bit() == 1) x = 2; 
      delay_us(120); 
      if (read_bit() == 1) x |= 1;   // And it's complement 

      if (x == 3)                   // There are no devices on the one wire bus 
      break; 
      else 
      { 
         if (x > 0)                 // All devices coupled have 0 or 1 
            g = x >> 1;             // Bit write value for search 

         // If this discrepancy is before the last discrepancy on a previous 
         // Next then pick the same as last time. 
         else 
         { 
            if (m < lastDiscrep) 
               g = ((RomBytes[n] & k) > 0); 
            // If equal to last pick 1 
            else 
               g = (m == lastDiscrep);  // If not then pick 0 

               // If 0 was picked then record position with mask k 
               if (g == 0) discrepMarker = m; 
         } 

         // Isolate bit in ROM[n] with mask k 
         if (g == 1) RomBytes[n] |= k; 
         else RomBytes[n] &= ~k; 

         write_bit(g);  // ROM search write 

         m++;           // Increment bit counter m 
         k = k << 1;    // and shift the bit mask k 
         // If the mask is 0 then go to new ROM 
         if (k == 0) 
         {  // Byte n and reset mask 
            ow_crc(RomBytes[n]);      // Accumulate the crc 
            n++; 
            k++; 
         } 
      } 
   } while (n < 8);  // Loop through until through all ROM bytes 0-7 

   if (m < (65||dowcrc))   // If search was unsuccessful then 
      lastDiscrep = 0;     // reset the last Discrepancy to zero 

   else  // Search was successful, so set lastDiscrep, lastOne, nxt 
   { 
      lastDiscrep = discrepMarker; 
      doneFlag = (lastDiscrep == 0); 
      nxt = TRUE; // Indicates search not yet complete, more parts remain 
   } 

   return nxt; 
} 
//****************************************************************************** 
// Resets current state of a ROM search and calls Next to find the first device 
// on the one wire bus. 
int8 First(void) 
{ 
   lastDiscrep = 0; 
   doneFlag = FALSE; 
   return Next();    // Call Next and return it's return value; 
} 
//****************************************************************************** 
void FindDevices(void) 
{ 
   int8 m; 

   if(!ow_reset()) 
   { 
      if(First())    // Begins when at least one part found 
      { 
         numROMs = 0; 

         do 
         { 
            numROMs++; 

            for (m=0;m<8;m++) 
            { 
               FoundROM[numROMs][m] = RomBytes[m]; 
            } 

            printf("Device No.%u address ",numROMs); 

            printf("%X%X%X%X%X%X%X%X\n\r", 
            FoundROM[numROMs][7],FoundROM[numROMs][6],FoundROM[numROMs][5], 
            FoundROM[numROMs][4],FoundROM[numROMs][3],FoundROM[numROMs][2], 
            FoundROM[numROMs][1],FoundROM[numROMs][0]); 

         } while (Next() && (numROMs<10));   // Continues until no additional 
                                             // devices found. 
      } 
   } 
   putc('\n'); putc('\r'); 
} 
//****************************************************************************** 
// Sends Match ROM command to bus then device address 
int8 Send_MatchRom(void) 
{ 
   int8 i; 
   if (ow_reset()) return FALSE;          // 0 if device present 
   write_byte(0x55);                      // Match ROM 

   for (i=0;i<8;i++) 
   { 
      write_byte(FoundRom[numROMs][i]);   // Send ROM code 
   } 

   return TRUE; 
} 

float ds1820_read() 
{ 
   signed int16 temperature; 
   int8 i, scratch[9], try=3;        

   while(try--) 
   { 
      output_float(TEMP_GND_PIN);       // Set as input. 4k7 pullup on bus. 
      
      ow_reset(); 
      write_byte(0xCC); // Skip Rom command 
      write_byte(0x44); // Temperature Convert command 
      output_float(TEMP_GND_PIN); 
      delay_ms(750);    // Max. time for conversion is 750mS 
      ow_reset(); 
      write_byte(0xCC); // Skip Rom command 
      write_byte(0xBE); // Read scratch pad command 
    
      dowcrc = 0;        
    
      // Get the data bytes 
      for (i=0;i<=7;i++) 
      { 
          scratch[i] = read_byte(); 
          ow_crc(scratch[i]);          
      } 
    
      scratch[8] = read_byte();   // Get crc byte 
      ow_reset(); 
      
      // If the received crc is same as calculated then data is valid. 
      // Scale and round to nearest degree C. 
      // Scaling is 0.0625 (1/16) deg.C/bit with default 12 bit resolution. 
      // Round by adding half denominator for positive temperatures and 
      // subtracting half denominator for negative temperatures. 

      if (scratch[8] == dowcrc) 
      { 
         float32 t; 
         temperature = (signed int16) make16(scratch[1],scratch[0]);    
         if (temperature >= 0) 
            t = ((float32)temperature + 8.0)/16.0; 
         else 
            t = ((float32)temperature - 8.0)/16.0; 
         return (t); 
      } 
   } 
    
   return DS1820_CONN_ERROR; 
} 