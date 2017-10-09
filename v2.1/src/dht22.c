#ifndef DHT_DATA
  #define DHT_DATA    PIN_C1
#endif

unsigned int8 dht22_data[5];

void dht22_start(void) {
  output_low(DHT_DATA);
  delay_ms(20);
  output_high(DHT_DATA);
  delay_us(30);
}

unsigned int8 dht_22_get_byte() {
   unsigned int8 i, result, timeout;
   i = result = timeout = 0;
   for (i=0; i < 8; i++) {
      while (input(DHT_DATA) == 0 && timeout < 255) {
         delay_us(2);
         timeout++;
      }
      delay_us(30);
      if (input(DHT_DATA) == 1) {
         result |=(1<<(7-i));
      }
      timeout = 0;
      while (input(DHT_DATA) == 1 && timeout < 255) {
         delay_us(2);
         timeout++;
      }
   }
   return result;
}

unsigned int1 dht22_read(float32 &humidity, float32 &temperature) {
   unsigned int8 i, dht22_checksum;
   unsigned int16 temperature16, humidity16;
   float32 temp,hum;

   output_high(DHT_DATA);
   delay_us(20);
   output_low(DHT_DATA);
   delay_ms(20);
   output_high(DHT_DATA);
   delay_us(30);

   if (input(DHT_DATA)) {
      temperature = humidity = 0;
      return 0;
   }

   delay_us(80);

   if (!input(DHT_DATA)) {
      temperature = humidity = 0;
      return 0;
   }

   delay_us(80);

   for (i = 0; i < 5; i++) {
      dht22_data[i] = dht_22_get_byte();
   }

   delay_us(10);
   output_high(DHT_DATA);

   dht22_checksum= dht22_data[0] + dht22_data[1] + dht22_data[2] + dht22_data[3];

   if (dht22_data[4] != dht22_checksum)
      return 0;

   humidity16 = make16(dht22_data[0], dht22_data[1]);
   temperature16 = make16(dht22_data[2], dht22_data[3]);

   hum = humidity16;
   temp = temperature16;
   
   humidity = (hum) / 10;
   temperature = (temp) / 10;

   return 1;
}