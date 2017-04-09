/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/

//TO DO
//4 fdd

//I/O devices flag
const uint8_t ACK = 0x06;
const uint16_t CPM_LBL_START = 0x18;
const uint16_t CPM_LBL_LEN = 36;
const uint16_t CPM_SERIAL_START = 0x328;
const uint16_t CPM_SERIAL_LEN = 6;

uint32_t blk;

void _charOut(uint8_t c) {
    out_port(SIOA_CON_PORT_DATA, c);
}

void _BIOS_RET() {
    uint16_t a16;
    //RET
    _AB = _SP;
    _RDMEM();
    a16 = _DB;
    _SP++;
    _AB = _SP;
    _RDMEM();
    a16 = a16 + 256 * _DB;
    _SP++;
    _PC = a16;
    _AB = _PC;
}

#define CPMSYS_COUNT 11
#define CPMSYS_LEN 5632
#define CPMSYS_START 0x100
#define CPMSYS_CS 0x1A

boolean _IPL() {
  uint16_t i;
  uint16_t j;
  uint16_t k;
  uint8_t checksum = 0x00;
  uint8_t res;
  uint8_t d8;
  boolean success = false;
  Serial.print(RAM_SIZE, DEC);
  Serial.println("K SYSTEM");
  Serial.print("CBASE: ");
  Serial.write(0x09);
  Serial.println(CBASE, HEX); 
  Serial.print("FBASE: ");
  Serial.write(0x09);
  Serial.println(FBASE, HEX);
  Serial.print("BIOS: ");
  Serial.write(0x09);
  Serial.print(_BIOS_LO, HEX);
  Serial.print(" ... ");
  Serial.println(_BIOS_HI, HEX);  
  _SP = SP_INIT;
  Serial.println("");
  Serial.println(F("IPL"));
  FDD_REG_DRV = 0;
  FDD_REG_TRK = 0;
  FDD_REG_SEC = 1;

  //reading from SD
  for(j=0;j<CPMSYS_COUNT;j++) {
      for(k=0;k<512;k=k+SD_BLK_SIZE) {
        res = readSD(j+CPMSYS_START, k);
        for (i = 0 ; i < SD_BLK_SIZE ; i++) {
          _dsk_buffer[i] = _buffer[i];
        }     
        for (i = 0 ; i < SD_BLK_SIZE ; i++) {
          _AB = CBASE+i+k+j*512;
          _DB = _dsk_buffer[i];
          _WRMEM();
        }   
      }  
      out_port(SIOA_CON_PORT_DATA, '.');
  }
  
  //checksum checking
  for(j=0;j<CPMSYS_COUNT;j++) {
      for (i = 0 ; i < 512 ; i++) {
        _AB = CBASE+i+j*512;
        _RDMEM();
        d8 = _DB;
        checksum = checksum + d8;
      }
  }
  Serial.println("");
  Serial.print(F("Checksum: "));
  sprintf(hex, "%02X", checksum);
  Serial.print(hex);
  Serial.print(" ");
  if (checksum != CPMSYS_CS) {
     Serial.println(F("ERR!"));
     success = false;
  }
  else {
     Serial.println(F("O.K.!"));
     success = true;

     for(j=CPM_LBL_START;j<(CPM_LBL_START+CPM_LBL_LEN);j++) {
        _AB = CBASE+j;
        _RDMEM();
        Serial.write(_DB);
     }
     Serial.println("");
     Serial.print(F("Serial: "));
     for(j=CPM_SERIAL_START;j<(CPM_SERIAL_START+CPM_SERIAL_LEN);j++) {
        _AB = CBASE+j;
        _RDMEM();
        sprintf(hex, "%02X", _DB);
        Serial.print(hex);
     }
     Serial.println("");
  
    i = 0;
    i = i + _DPBASE;      
    _AB = i + 0;
    _DB = 0x00;
    _WRMEM();
    _AB = i + 1;
    _DB = 0x00;
    _WRMEM();
    _AB = i + 2;
    _DB = 0x00;
    _WRMEM();
    _AB = i + 3;
    _DB = 0x00;
    _WRMEM();
    _AB = i + 4;
    _DB = 0x00;
    _WRMEM();
    _AB = i + 5;
    _DB = 0x00;
    _WRMEM();
    _AB = i + 6;
    _DB = 0x00;
    _WRMEM();
    _AB = i + 7;
    _DB = 0x00;
    _WRMEM();
    //DIRBUF
    _AB = i + 8;
    _DB = lowByte(_DIRBUF);
    _WRMEM();
    _AB = i + 9;
    _DB = highByte(_DIRBUF);
    _WRMEM();
    //DPB
    _AB = i + 10;
    _DB = lowByte(_DPBLK);
    _WRMEM();
    _AB = i + 11;
    _DB = highByte(_DPBLK);
    _WRMEM();
    //CSV
    _AB = i + 12;
    _DB = lowByte(_CHK00);
    _WRMEM();
    _AB = i + 13;
    _DB = highByte(_CHK00);
    _WRMEM();
    //ALV
    _AB = i + 14;
    _DB = lowByte(_ALL00);
    _WRMEM();
    _AB = i + 15;
    _DB = highByte(_ALL00);
    _WRMEM();
    //DPB init
    i = _DPBLK;
    //SPT
    _AB = i;
    _DB = 26;
    _WRMEM();
    _AB = i + 1;
    _DB = 0;
    _WRMEM();
    //BSH
    _AB = i + 2;
    _DB = 3;
    _WRMEM();
    //BLM
    _AB = i + 3;
    _DB = 7;
    _WRMEM();
    //EXM
    _AB = i + 4;
    _DB = 0;
    _WRMEM();
    //DSM
    _AB = i + 5;
    _DB = 242;
    _WRMEM();
    _AB = i + 6;
    _DB = 0;
    _WRMEM();
    //DRM
    _AB = i + 7;
    _DB = 63;
    _WRMEM();
    _AB = i + 8;
    _DB = 0;
    _WRMEM();
    //AL0    
    _AB = i + 9;
    _DB = 192;
    _WRMEM();
    //AL1
    _AB = i + 0xA;
    _DB = 0;
    _WRMEM();
    //CKS
    _AB = i + 0xB;
    _DB = 0;
    _WRMEM();
    _AB = i + 0xC;
    _DB = 0;
    _WRMEM();
    //OFF
    _AB = i + 0xD;
    _DB = 0;
    _WRMEM();
    _AB = i + 0xE;
    _DB = 0;
    _WRMEM();
    _AB = i + 0xF;
    _DB = 0;
    _WRMEM();
  /*      
     BLS       BSH     BLM           EXM
   -----      ---     ---     DSM<256   DSM>=256
    1024       3       7         0        n/a 
   */
    }
    return success;
}

void _GOCPM(boolean jmp) {
    //JMP TO WBOOT
    _AB = JMP_BOOT;
    _DB = 0xC3;
    _WRMEM();
    _AB = JMP_BOOT + 1;
    _DB = lowByte(_BIOS+3);
    _WRMEM();
    _AB = JMP_BOOT + 2;
    _DB = highByte(_BIOS+3);
    _WRMEM();
    //JMP TO BDOS
    _AB = JMP_BDOS;
    _DB = 0xC3;
    _WRMEM();
    _AB = JMP_BDOS + 1;
    _DB = lowByte(FBASE);
    _WRMEM();
    _AB = JMP_BDOS + 2;
    _DB = highByte(FBASE);
    _WRMEM();
    //SETDMA 0x80
    _AB = word(FDD_PORT_DMA_ADDR_LO, FDD_PORT_DMA_ADDR_LO);
    _DB = 0x80;
    _OUTPORT();
    _AB = word(FDD_PORT_DMA_ADDR_HI, FDD_PORT_DMA_ADDR_HI);
    _DB = 0x00;
    _OUTPORT();
    //GET CURRENT DISK NUMBER   SEND TO THE CCP
    _AB = CDISK;
    _RDMEM();
    _rC = _DB;
    //GO TO CP/M FOR FURTHER PROCESSING
    if (jmp) { 
      _PC = CBASE;
      _AB = _PC;
    }
}

void _BOOT() {
    //message BOOT
    Serial.println("");
    Serial.println(F("BOOT"));
    _AB = IOBYTE;
    _DB = 0x00;
    _WRMEM();//IOBYTE clear
    _AB = CDISK;
    _DB = 0x00;
    _WRMEM();//select disk 0
    //INITIALIZE AND GO TO CP/M
    _GOCPM(true);
}

void _WBOOT() {
  boolean load;
    //message WBOOT
    Serial.println("");
    Serial.println(F("WBOOT"));
    //USE SPACE BELOW BUFFER FOR STACK
    _SP = 0x80;
    do {
      load = _IPL();
    } while (!load);
    //INITIALIZE AND GO TO CP/M
    _GOCPM(true);
}

void _BIOS_BOOT() {
    _BOOT();
}


void _BIOS_WBOOT() {
    _WBOOT();
}


void _BIOS_CONST() {
     _AB = word(SIOA_CON_PORT_STATUS, SIOA_CON_PORT_STATUS);
     _INPORT();
     if ((_DB & 0x20)!=0) {
          _rA = 0xFF;
     }
     else {
          _rA = 0x00;
    }
    _BIOS_RET();
}


void _BIOS_CONIN() {
      _AB = word(SIOA_CON_PORT_DATA, SIOA_CON_PORT_DATA);
      _INPORT();
      _rA = _DB & B01111111;
      _BIOS_RET();
}


void _BIOS_CONOUT() {
    _AB = SIOA_CON_PORT_DATA;
    _DB = _rC;
    _OUTPORT();
    _BIOS_RET();
}

void _BIOS_LIST() {
    _BIOS_RET();    
}

void _BIOS_PUNCH() {
    _BIOS_RET();    
}

void _BIOS_READER() {
      _AB = word(SIOA_CON_PORT_DATA, SIOA_CON_PORT_DATA);
      _INPORT();
      _rA = _DB & B01111111;
       //ACK sent
       _AB = word(SIOA_CON_PORT_DATA, SIOA_CON_PORT_DATA);
       _DB = ACK;
       _OUTPORT();
      _BIOS_RET();    
}

void _BIOS_LISTST() {

    _BIOS_RET();    
}

void _BIOS_SETTRK() {
     _AB = word(FDD_PORT_TRK, FDD_PORT_TRK);
     _DB = _rC;
     _OUTPORT();
    _BIOS_RET();    
}

void _BIOS_HOME() {
    _rC = 0;//track 0
    _AB = word(FDD_PORT_TRK, FDD_PORT_TRK);
    _DB = _rC;
    _OUTPORT();
    _BIOS_RET();        
}

void _BIOS_SELDSK() {
     uint16_t a16;
     uint8_t d8;
     d8 = _rC;
      if (d8>(FDD_NUM-1)) {
        _rH = 0;
        _rL = 0;  
      }
      else {
      _AB = word(FDD_PORT_DRV, FDD_PORT_DRV);
      _DB = d8;
      _OUTPORT();
      a16 = d8 * 16;
      a16 = a16 + _DPBASE;
      _rH = highByte(a16);
      _rL = lowByte(a16);
     }
    _BIOS_RET();    
}

void _BIOS_SETSEC() {
    _AB = word(FDD_PORT_SEC, FDD_PORT_SEC);
    _DB = _rC;
    _OUTPORT();
    _BIOS_RET();    
}

void _BIOS_SETDMA() {
    _AB = word(FDD_PORT_DMA_ADDR_LO, FDD_PORT_DMA_ADDR_LO);
    _DB = _rC;
    _OUTPORT();
    _AB = word(FDD_PORT_DMA_ADDR_HI, FDD_PORT_DMA_ADDR_HI);
    _DB = _rB;
    _OUTPORT();
    _BIOS_RET();    
}

void _BIOS_READ() {
     _AB = word(FDD_PORT_CMD, FDD_PORT_CMD);
    _DB = FDD_RD_CMD;
    _OUTPORT();
    _AB = word(FDD_PORT_CMD, FDD_PORT_CMD);
    _INPORT();
     if (_DB) { 
      _rA = DISK_SUCCESS;
     }
     else {
      _rA = DISK_ERROR;
     }
    _BIOS_RET();    
}

void _BIOS_WRITE() {
    _AB = word(FDD_PORT_CMD, FDD_PORT_CMD);
    _DB = FDD_WRT_CMD;
    _OUTPORT();
    _AB = word(FDD_PORT_CMD, FDD_PORT_CMD);
    _INPORT();
    if (_DB) { 
      _rA = DISK_SUCCESS;
     }
     else {
      _rA = DISK_ERROR;
     }
    _BIOS_RET();    
}

void _BIOS_SECTRAN() {
     //_Regs[_Reg_C] -> logical sector (from 0)
     _rL = _rC+1; 
     _rH = 0;
    _BIOS_RET();    
}

