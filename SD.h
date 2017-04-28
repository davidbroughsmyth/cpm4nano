
//SD
Sd2Card card;
const uint8_t SS_SD_pin = 10;//SS pin (D10)
const uint16_t SD_BLK_SIZE = 128;//SD block size
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
