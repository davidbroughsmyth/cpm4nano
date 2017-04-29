/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/

Sd2Card card;
const uint8_t SS_SD_pin = 10;//SS pin (D10)
const uint16_t SD_BLK_SIZE = 128;//SD block size

//SD buffers
static unsigned char _buffer[SD_BLK_SIZE];
static unsigned char _dsk_buffer[SD_BLK_SIZE];

//block read from SD
uint8_t readSD (uint32_t blk, uint16_t offset) {
  uint8_t res;
  res = card.readBlock(blk, _buffer, offset);
  return res;
}

//block write to SD
uint8_t writeSD (uint32_t blk) {
  uint8_t res;
  res = card.writeBlock(blk, _buffer);
  if (!LED_on) {
    fastDigitalWrite(LED_pin, HIGH);
    LED_on = true;//WRITE LED on
  }
  LED_count = LED_delay;
  return res;
}

//erase SD
uint8_t eraseSD (uint32_t blk, uint32_t len) {
  uint8_t res;
  res = card.erase(blk, blk+len-1);
  return res;
}

