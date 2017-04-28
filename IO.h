/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/

//address <- _AB
//data -> _DB

void _INPORT() {  
  uint8_t dat;
  boolean readyFlag = false;
  dat = 0x00;
  switch (lowByte(_AB)) {
    //SIO-A
    case SIOA_CON_PORT_STATUS:
      //bit 1 - ready to out (Altair)
      //bit 5 - ready to in (Altair)
      //bit 7 - ready to out (IMSAI)
      //bit 0 - ready to in (IMSAI)
      dat = 0x02 | 0x80;
      if (con_ready()) {
        dat = dat | 0x20;
        dat = dat | 0x01;
      }
      break;
    case SIOA_CON_PORT_DATA:
      //input from console
      do {
        if (con_ready()) {
          dat = uint8_t(con_read());
          readyFlag = true;
        }
      } while (!readyFlag);
      break;
    //SIO-2
    case SIO2_CON_PORT_STATUS:
      //bit 1 - ready to out (Altair)
      //bit 0 - ready to in (Altair)
      dat = 0x02;
      if (con_ready()) {
        dat = dat | 0x01;
      }
      break;
    case SIO2_CON_PORT_DATA:
      //input from console
      do {
        if (con_ready()) {
          dat = uint8_t(con_read());
          readyFlag = true;
        }
      } while (!readyFlag);
      break;
    //FDD ports
    case FDD_PORT_CMD:
      //status
      if (FDD_REG_STATUS) {
        dat = 1;
      }
      else {
        dat = 0;
      }
      break;
    case FDD_PORT_TRK:
      //track
      dat = FDD_REG_TRK;
      break;
    case FDD_PORT_SEC:
      //sector
      dat = FDD_REG_SEC;
      break;
    case FDD_PORT_DRV:
      //drive select
      dat = FDD_REG_DRV;
      break;
    case SENSE_SW_PORT:
      //Altair/IMSAI sense switch
      dat = SENSE_SW;
      break;
    //MMU registers
    case MMU_BLOCK_SEL_PORT:
      dat = MMU_BLOCK_SEL_REG;
      break;
    case MMU_BANK_SEL_PORT:
      dat = bank_get(MMU_BLOCK_SEL_REG);
      break;
    case IN_PORT:
      dat = digitalRead(IN_pin);
      if ((dat && 0x01) == 0x01) {
        dat = 0x01;
      }
      else {
        dat = 0x00;
      }
      break;
  }
  _DB = dat;
}

//address <- _AB
//data <- _DB
void _OUTPORT() {
  uint8_t dat;
  uint16_t i;
  uint8_t res;
  uint32_t blk;
  dat = _DB;
  switch (lowByte(_AB)) {
    //console ports
    //SIO-A
    case SIOA_CON_PORT_DATA:
      //output to console
      Serial.write(dat);
      break;
    //SIO-2
    case SIO2_CON_PORT_DATA:
      //output to console
      Serial.write(dat);
      break;
    //FDD ports
    case FDD_PORT_CMD:
      //command
      if (dat == FDD_RD_CMD) {
        //sector read
        //blk = _getMEM(_FDD_SECTOR)-1;
        blk = FDD_REG_SEC - 1L;
        blk = blk + FDD_REG_TRK * TRACK_SIZE;
        blk = blk +  SD_FDD_OFFSET[FDD_REG_DRV];
        res = readSD(blk, 0);
        if (res == 1) {
          for (i = 0 ; i < SD_BLK_SIZE ; i++) {
            _dsk_buffer[i] = _buffer[i];
          }
          for (i = 0; i < SD_BLK_SIZE; i++) {
            _setMEM(FDD_REG_DMA + i, _dsk_buffer[i]);
          }
          FDD_REG_STATUS = true;
        }
        else {
          FDD_REG_STATUS = false;
        }
      }
      if (dat == FDD_WRT_CMD) {
        //sector write
        blk = FDD_REG_SEC - 1L;
        blk = blk + FDD_REG_TRK * TRACK_SIZE;
        blk = blk +  SD_FDD_OFFSET[FDD_REG_DRV];
        for (i = 0 ; i < SD_BLK_SIZE ; i++) {
          _dsk_buffer[i] = _getMEM(FDD_REG_DMA + i);
        }
        for (i = 0; i < SD_BLK_SIZE; i++) {
          _buffer[i] = _dsk_buffer[i];
        }
        res = writeSD(blk);
        if (res == 1) {
          FDD_REG_STATUS = true;
        }
        else {
          FDD_REG_STATUS = false;
        }
      }
      break;
    case FDD_PORT_TRK:
      //track
      FDD_REG_TRK = dat;
      break;
    case FDD_PORT_SEC:
      //sector
      FDD_REG_SEC = dat;
      break;
    case FDD_PORT_DRV:
      //drive select
      FDD_REG_DRV = dat;
      break;
    case FDD_PORT_DMA_ADDR_LO:
      FDD_REG_DMA = FDD_REG_DMA & 0xFF00;
      FDD_REG_DMA = FDD_REG_DMA | dat;
      break;
    case FDD_PORT_DMA_ADDR_HI:
      FDD_REG_DMA = FDD_REG_DMA & 0x00FF;
      FDD_REG_DMA = FDD_REG_DMA + dat * 256;
      break;
    //MMU registers
    case MMU_BLOCK_SEL_PORT:
      MMU_BLOCK_SEL_REG = dat;
      break;
    case MMU_BANK_SEL_PORT:
      bank_set(MMU_BLOCK_SEL_REG,dat);
      break;
    case OUT_PORT:
      //bit 0 out
      if ((dat && 0x01) == 0x01) {
        fastDigitalWrite(OUT_pin, HIGH);
      }
      else {
        fastDigitalWrite(OUT_pin, LOW);
      }
      break;
  }
}

uint8_t _getPORT(uint16_t adr) {
  _AB = adr;
  _INPORT();
  return _DB;
}

void _setPORT(uint16_t adr, uint8_t dat) {
  _AB = adr;
  _DB = dat;
  _OUTPORT();
}