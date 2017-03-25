/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/

//TO DO
//четыре диска

//флаги устройств ввода-вывода
bool CON_IN = 0; //PUTTY
bool CON_OUT = 0; //PUTTY

const uint8_t ACK = 0x06;

uint32_t blk;

void _charOut(uint8_t c) {
    switch (CON_OUT) {
      case 0: //PUTTY
        Serial.write(c);
        break;
    }
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

#define CPMSYS_COUNT 15
#define CPMSYS_LEN 7680
#define CPMSYS_START 0x100
#define CPMSYS_CS 0xBC

boolean _IPL() {
  uint16_t i;
  uint16_t j;
  uint16_t k;
  uint8_t checksum = 0x00;
  uint8_t res;
  uint8_t d8;
  boolean success = false;
  _SP = SP_INIT;
  Serial.println(" ");
  _charOut('I');
  _charOut('P');
  _charOut('L');
  _charOut(0x0D);
  _charOut(0x0A);
  Serial.println(F("Starting CP/M loading..."));
  
  //reading from FLASH
  /*
  for(j=0;j<CPMSYS_LEN;j++) {
    d8 = pgm_read_byte_near(CPM_BIN+j);
    _setMEM(j+CBASE, d8);
    if ((j % 1024) == 0) {
      Serial.print(".");
    }
  }
  */
  _setMEM(_DISKNO, 0);
  _setMEM(_TRACK, 0);
  _setMEM(_SECTOR, 1);

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
      Serial.print(".");   
  }
  
  //checksum checking
  for(j=0;j<CPMSYS_COUNT;j++) {
      for (i = 0 ; i < 512 ; i++) {
        d8 = _getMEM(CBASE+i+j*512);
        checksum = checksum + d8;
      }
  }
  Serial.println("");
  Serial.print(F("Checksum: "));
  Serial.println(checksum, HEX);
  if (checksum != CPMSYS_CS) {
     Serial.println(F("Checksum error!!!"));
     success = false;
  }
  else {
  Serial.println(F("Checksum O.K."));
  success = true;
  
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
    _setMEM(_DMAAD, 0x80);
    _setMEM(_DMAAD+1, 0x00);
    //GET CURRENT DISK NUMBER   SEND TO THE CCP
    _Regs[_Reg_C] = _getMEM(CDISK);
    //GO TO CP/M FOR FURTHER PROCESSING
    if (jmp) { 
      _PC = CBASE;
    }
}

void _BOOT() {
    //message BOOT
    Serial.println(" ");
    _charOut('B');
    _charOut('O');
    _charOut('O');
    _charOut('T');
    _charOut(0x0D);
    _charOut(0x0A);
    //IOBYTE clear
    _setMEM(IOBYTE, 0x00);
    //select disk 0
    _setMEM(CDISK, 0x00);
    //INITIALIZE AND GO TO CP/M
    _GOCPM(true);
}

void _WBOOT() {
  boolean load;
    do {
    //message WBOOT
    Serial.println(" ");
    _charOut('W');
    _charOut('B');
    _charOut('O');
    _charOut('O');
    _charOut('T');
    _charOut(0x0D);
    _charOut(0x0A);
    //USE SPACE BELOW BUFFER FOR STACK
    _SP = 0x80;
    load = _IPL();
    if (load) {
    Serial.println(F("CP/M loading successfull"));
    }
    else {
      Serial.println(F("CP/M loading failed!"));
    }
    } while (!load);
    //INITIALIZE AND GO TO CP/M
    _GOCPM(true);
}

void _BIOS_BOOT() {
    _BOOT();
    //_BIOS_RET();  ???    
}


void _BIOS_WBOOT() {
    _WBOOT();
    //_BIOS_RET(); ???
}


void _BIOS_CONST() {
    switch (CON_IN) {
      case 0: //PUTTY
        if (Serial.available() > 0) {
          _Regs[_Reg_A] = 0xFF;
        }
        else {
          _Regs[_Reg_A] = 0x00;
        }
        break;
    }
    _BIOS_RET();
}


void _BIOS_CONIN() {
    bool flag = false;
    char charIn;
    switch (CON_IN) {
      case 0: //PUTTY
        do
        {
          if (Serial.available() > 0) {
            _Regs[_Reg_A] = Serial.read();
            _Regs[_Reg_A] = _Regs[_Reg_A] & B01111111;
            flag = true;
          }
        } while (!flag);
        break;
      }    
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
      bool flag = false;
      do
        {
          if (Serial.available() > 0) {
            _Regs[_Reg_A] = Serial.read();
            _Regs[_Reg_A] = _Regs[_Reg_A] & B01111111;
            flag = true;
          }
        } while (!flag);
        //ACK sent
        Serial.write(ACK);
      _BIOS_RET();    
}

void _BIOS_LISTST() {

    _BIOS_RET();    
}

void _BIOS_SETTRK() {
     _setMEM(_TRACK, _Regs[_Reg_C]);
    _BIOS_RET();    
}

void _BIOS_HOME() {
    _Regs[_Reg_C] = 0;//track 0
    _setMEM(_TRACK, _Regs[_Reg_C]);
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
      _setMEM(_DISKNO, d8);
      a16 = d8 * 16;
      a16 = a16 + _DPBASE;
      _Regs[_Reg_H] = highByte(a16);
      _Regs[_Reg_L] = lowByte(a16);
     }
    _BIOS_RET();    
}

void _BIOS_SETSEC() {
    _setMEM(_SECTOR, _Regs[_Reg_C]); 
    _BIOS_RET();    
}

void _BIOS_SETDMA() {
    _setMEM(_DMAAD, _Regs[_Reg_C]);
    _setMEM(_DMAAD+1, _Regs[_Reg_B]);
    _BIOS_RET();    
}

void _BIOS_READ() {
     int i;
     uint8_t res;
     blk = _getMEM(_SECTOR)-1;
     blk = blk + _getMEM(_TRACK)*TRACK_SIZE; 
     switch (_getMEM(_DISKNO)) {
      case 0:
         blk = blk +  SD_FDD_A_OFFSET;
         break;
     }
     res = readSD(blk, 0);
     if (res==1) {
     for (i = 0 ; i < SD_BLK_SIZE ; i++) {
        _dsk_buffer[i] = _buffer[i];
      }
     for(i=0;i<SD_BLK_SIZE;i++) {
      _setMEM(_getMEM(_DMAAD)+_getMEM(_DMAAD+1)*256+i,_dsk_buffer[i]); 
     }
     _Regs[_Reg_A] = DISK_SUCCESS;
     }
     else {
      _Regs[_Reg_A] = DISK_ERROR;
     }
    _BIOS_RET();    
}

void _BIOS_WRITE() {
    int i;
    uint8_t res; 
     blk = _getMEM(_SECTOR)-1;
     blk = blk + _getMEM(_TRACK)*TRACK_SIZE; 
     switch (_getMEM(_DISKNO)) {
      case 0:
         blk = blk +  SD_FDD_A_OFFSET;
         break;
     } 
    for (i = 0 ; i < SD_BLK_SIZE ; i++) {
        _dsk_buffer[i] = _getMEM(_getMEM(_DMAAD)+_getMEM(_DMAAD+1)*256+i);
    }
    for(i=0;i<SD_BLK_SIZE;i++) {
      _buffer[i] = _dsk_buffer[i]; 
    }
     res = writeSD(blk);
     if (res==1) {
      _Regs[_Reg_A] = DISK_SUCCESS;
     }
     else {
      _Regs[_Reg_A] = DISK_ERROR;
     }
    _BIOS_RET();    
}

void _BIOS_SECTRAN() {
     _Regs[_Reg_L] = _Regs[_Reg_C]; 
     _Regs[_Reg_H] = 0;
    _BIOS_RET();    
}

