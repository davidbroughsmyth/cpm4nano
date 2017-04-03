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
    a16 = _getMEM(_SP);
    _SP++;
    a16 = a16 + 256 * _getMEM(_SP);
    _SP++;
    _PC = a16;
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
  out_port(SIOA_CON_PORT_DATA, 0x0D);  
  out_port(SIOA_CON_PORT_DATA, 0x0A);
  out_port(SIOA_CON_PORT_DATA, 'I');
  out_port(SIOA_CON_PORT_DATA, 'P');
  out_port(SIOA_CON_PORT_DATA, 'L');
  out_port(SIOA_CON_PORT_DATA, 0x0D);
  out_port(SIOA_CON_PORT_DATA, 0x0A);
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
          _setMEM(CBASE+i+k+j*512, _dsk_buffer[i]);
        }   
      }  
      out_port(SIOA_CON_PORT_DATA, '.');
  }
  
  //checksum checking
  for(j=0;j<CPMSYS_COUNT;j++) {
      for (i = 0 ; i < 512 ; i++) {
        d8 = _getMEM(CBASE+i+j*512);
        checksum = checksum + d8;
      }
  }
  out_port(SIOA_CON_PORT_DATA, 0x0D);  
  out_port(SIOA_CON_PORT_DATA, 0x0A);
  Serial.print(F("Checksum: "));
  sprintf(hex, "%02X", checksum);
  Serial.print(hex);
  Serial.print(" ");
  if (checksum != CPMSYS_CS) {
     out_port(SIOA_CON_PORT_DATA, 'E');
     out_port(SIOA_CON_PORT_DATA, 'R');
     out_port(SIOA_CON_PORT_DATA, 'R');
     out_port(SIOA_CON_PORT_DATA, '!');
     out_port(SIOA_CON_PORT_DATA, 0x0D);  
     out_port(SIOA_CON_PORT_DATA, 0x0A);
     success = false;
  }
  else {
     out_port(SIOA_CON_PORT_DATA, 'O');
     out_port(SIOA_CON_PORT_DATA, '.');
     out_port(SIOA_CON_PORT_DATA, 'K');
     out_port(SIOA_CON_PORT_DATA, '.');
     out_port(SIOA_CON_PORT_DATA, 0x0D);  
     out_port(SIOA_CON_PORT_DATA, 0x0A);
     success = true;

     for(j=CPM_LBL_START;j<(CPM_LBL_START+CPM_LBL_LEN);j++) {
        Serial.write(_getMEM(CBASE+j));
     }
     Serial.println("");
     Serial.print(F("Serial: "));
     for(j=CPM_SERIAL_START;j<(CPM_SERIAL_START+CPM_SERIAL_LEN);j++) {
        sprintf(hex, "%02X", _getMEM(CBASE+j));
        Serial.print(hex);
     }
     Serial.println("");
  
    i = 0;
    i = i + _DPBASE;      
    _setMEM(i + 0, 0x00);
    _setMEM(i + 1, 0x00);
    _setMEM(i + 2, 0x00);
    _setMEM(i + 3, 0x00);
    _setMEM(i + 4, 0x00);
    _setMEM(i + 5, 0x00);
    _setMEM(i + 6, 0x00);
    _setMEM(i + 7, 0x00);
    //DIRBUF
    _setMEM(i + 8, lowByte(_DIRBUF));
    _setMEM(i + 9, highByte(_DIRBUF));
    //DPB
    _setMEM(i + 10, lowByte(_DPBLK));
    _setMEM(i + 11, highByte(_DPBLK));
    //CSV
    _setMEM(i + 12, lowByte(_CHK00));
    _setMEM(i + 13, highByte(_CHK00));
    //ALV
    _setMEM(i + 14, lowByte(_ALL00));
    _setMEM(i + 15, highByte(_ALL00));
    //DPB init
    i = _DPBLK;
    //SPT
    _setMEM(i, 26);
    _setMEM(i + 1, 0);
    //BSH
    _setMEM(i + 2, 3);
    //BLM
    _setMEM(i + 3, 7); 
    //EXM
    _setMEM(i + 4, 0);
    //DSM
    _setMEM(i + 5, 242); 
    _setMEM(i + 6, 0);
    //DRM
    _setMEM(i + 7, 63);  
    _setMEM(i + 8, 0);
    //AL0
    _setMEM(i + 9, 192);
    //AL1
    _setMEM(i + 0xA, 0);
    //CKS
    _setMEM(i + 0xB, 0);
    _setMEM(i + 0xC, 0);
    //OFF
    _setMEM(i + 0xD, 0);
    _setMEM(i + 0xE, 0);
    _setMEM(i + 0xF, 0);
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
    _setMEM(JMP_BOOT, 0xC3);
    _setMEM(JMP_BOOT+1, lowByte(_BIOS+3));
    _setMEM(JMP_BOOT+2, highByte(_BIOS+3));
    //JMP TO BDOS
    _setMEM(JMP_BDOS, 0xC3);
    _setMEM(JMP_BDOS+1, lowByte(FBASE));
    _setMEM(JMP_BDOS+2, highByte(FBASE));
    //SETDMA 0x80
    out_port(FDD_PORT_DMA_ADDR_LO, 0x80);
    out_port(FDD_PORT_DMA_ADDR_HI, 0x00);
    //GET CURRENT DISK NUMBER   SEND TO THE CCP
    _Regs[_Reg_C] = _getMEM(CDISK);
    //GO TO CP/M FOR FURTHER PROCESSING
    if (jmp) { 
      _PC = CBASE;
    }
}

void _BOOT() {
    //message BOOT
    out_port(SIOA_CON_PORT_DATA, 0x0D);
    out_port(SIOA_CON_PORT_DATA, 0x0A);
    out_port(SIOA_CON_PORT_DATA, 'B');
    out_port(SIOA_CON_PORT_DATA, 'O');
    out_port(SIOA_CON_PORT_DATA, 'O');
    out_port(SIOA_CON_PORT_DATA, 'T');
    out_port(SIOA_CON_PORT_DATA, 0x0D);
    out_port(SIOA_CON_PORT_DATA, 0x0A);
    //IOBYTE clear
    _setMEM(IOBYTE, 0x00);
    //select disk 0
    _setMEM(CDISK, 0x00);
    //INITIALIZE AND GO TO CP/M
    _GOCPM(true);
}

void _WBOOT() {
  boolean load;
    //message WBOOT
    out_port(SIOA_CON_PORT_DATA, 0x0D);
    out_port(SIOA_CON_PORT_DATA, 0x0A);
    out_port(SIOA_CON_PORT_DATA, 'W');
    out_port(SIOA_CON_PORT_DATA, 'B');
    out_port(SIOA_CON_PORT_DATA, 'O');
    out_port(SIOA_CON_PORT_DATA, 'O');
    out_port(SIOA_CON_PORT_DATA, 'T');
    out_port(SIOA_CON_PORT_DATA, 0x0D);
    out_port(SIOA_CON_PORT_DATA, 0x0A);
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
     if ((in_port(SIOA_CON_PORT_STATUS) & 0x20)!=0) {
          _Regs[_Reg_A] = 0xFF;
     }
     else {
          _Regs[_Reg_A] = 0x00;
    }
    _BIOS_RET();
}


void _BIOS_CONIN() {
      _Regs[_Reg_A] = in_port(SIOA_CON_PORT_DATA) & B01111111;
      _BIOS_RET();
}


void _BIOS_CONOUT() {
    _charOut(_Regs[_Reg_C]);
    _BIOS_RET();
}

void _BIOS_LIST() {
    _BIOS_RET();    
}

void _BIOS_PUNCH() {
    _BIOS_RET();    
}

void _BIOS_READER() {
      _Regs[_Reg_A] = in_port(SIOA_CON_PORT_DATA) & B01111111;
       //ACK sent
       out_port(SIOA_CON_PORT_DATA, ACK);
      _BIOS_RET();    
}

void _BIOS_LISTST() {

    _BIOS_RET();    
}

void _BIOS_SETTRK() {
     out_port(FDD_PORT_TRK,_Regs[_Reg_C]); 
    _BIOS_RET();    
}

void _BIOS_HOME() {
    _Regs[_Reg_C] = 0;//track 0
    out_port(FDD_PORT_TRK,_Regs[_Reg_C]); 
    _BIOS_RET();        
}

void _BIOS_SELDSK() {
     uint16_t a16;
     uint8_t d8;
     d8 = _Regs[_Reg_C];
      if (d8>(FDD_NUM-1)) {
        _Regs[_Reg_H] = 0;
        _Regs[_Reg_L] = 0;  
      }
      else {
      out_port(FDD_PORT_DRV, d8);
      a16 = d8 * 16;
      a16 = a16 + _DPBASE;
      _Regs[_Reg_H] = highByte(a16);
      _Regs[_Reg_L] = lowByte(a16);
     }
    _BIOS_RET();    
}

void _BIOS_SETSEC() {
    out_port(FDD_PORT_SEC,_Regs[_Reg_C]); 
    _BIOS_RET();    
}

void _BIOS_SETDMA() {
    out_port(FDD_PORT_DMA_ADDR_LO, _Regs[_Reg_C]);
    out_port(FDD_PORT_DMA_ADDR_HI, _Regs[_Reg_B]);
    _BIOS_RET();    
}

void _BIOS_READ() {
     uint8_t res;
     out_port(FDD_PORT_CMD,FDD_RD_CMD);
     if (in_port(FDD_PORT_CMD)) { 
     _Regs[_Reg_A] = DISK_SUCCESS;
     }
     else {
      _Regs[_Reg_A] = DISK_ERROR;
     }
    _BIOS_RET();    
}

void _BIOS_WRITE() {
    uint8_t res; 
    out_port(FDD_PORT_CMD,FDD_WRT_CMD);
    if (in_port(FDD_PORT_CMD)) { 
      _Regs[_Reg_A] = DISK_SUCCESS;
     }
     else {
      _Regs[_Reg_A] = DISK_ERROR;
     }
    _BIOS_RET();    
}

void _BIOS_SECTRAN() {
     //_Regs[_Reg_C] -> logical sector (from 0)
     _Regs[_Reg_L] = _Regs[_Reg_C]+1; 
     _Regs[_Reg_H] = 0;
    _BIOS_RET();    
}

