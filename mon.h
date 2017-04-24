/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/

//TO DO
//command length check

    clrarea();//clear work area
    
    //LXXXXYY - load byte to memory
    if (mon_buffer[0]=='L') {
      if (hexcheck(1,6)) {
        _AB = kbd2word(1);
        _DB = kbd2byte(5);
        _WRMEM();
        Serial.println("O.K.");
        goto MON_END;
      }
      else {
        goto MON_INVALID;
      }
    }

    //DXXXX - dump byte from memory
    if (mon_buffer[0]=='D') {
        if (hexcheck(1,4)) {
          _AB = kbd2word(1);
          _RDMEM();
          dat = _DB;
          Serial.println(dat, HEX);
          Serial.println(F("O.K."));
          goto MON_END;
        }
        else {
          goto MON_INVALID;
      }
    }

    //OXXYY - send byte to port
    if (mon_buffer[0]=='O') {
      if (hexcheck(1,4)) {
        port = kbd2byte(1);
        dat = kbd2byte(3);
        out_port(port, dat);
        Serial.println("O.K.");
        goto MON_END;
      }
      else {
        goto MON_INVALID;
      }
    }

    //IXX - read byte from port
    if (mon_buffer[0]=='I') {
        if (hexcheck(1,2)) {
          port = kbd2byte(1);
          dat = in_port(port);
          Serial.println(dat, HEX);
          Serial.println(F("O.K."));
          goto MON_END;
        }
        else {
          goto MON_INVALID;
      }
    }

    //F - load Intel HEX file to memory
    if (mon_buffer[0]=='F') {  
    Serial.println(F("Ready to receive the HEX file..."));  
    error = false;
    _EOF = false;
    hex_crc = 0;
    hex_count = 0;
    hex_bytes = 0;
    hex_type = 0;
    do {
      inChar = '\0';
      if (con_ready()) {
        inChar = con_read();       
        switch (hex_count) {
          case 0://:
              // : waiting
              if (inChar!=':') {
                break;//next char
              }
              else {
                hex_count++; 
                break; 
              }
           case 1:
              hex_len = chr2hex(inChar)*16;
              hex_count++;      
              break;
           case 2:
              hex_len = chr2hex(inChar) + hex_len;
              hex_crc = hex_crc + hex_len;
              hex_count++;      
              break;
           case 3:
              dat = chr2hex(inChar) *16;
              hex_count++;      
              break;
           case 4:
              dat = dat + chr2hex(inChar); 
              hex_crc = hex_crc + dat;
              adr = dat;      
              hex_count++;      
              break;
          case 5:
              dat = chr2hex(inChar)*16;       
              hex_count++;      
              break;
           case 6:
              dat = dat + chr2hex(inChar);  
              hex_crc = hex_crc + dat;
              adr = adr*256 + dat;     
              hex_count++;      
              break;
           case 7:
              dat = chr2hex(inChar)*16;
              hex_count++;      
              break;
           case 8:
              dat = dat + chr2hex(inChar); 
              hex_crc = hex_crc + dat;
              hex_type = dat;      
              hex_count++;      
              break;
           default:
              if (hex_count == (hex_len*2 + 10)) {
                //CRC
                dat = chr2hex(inChar) + dat*16;
                hex_crc = hex_crc ^ 0xFF;
                hex_crc++;
                if (hex_crc != dat) {
                  error = true;
                  Serial.println("");
                  Serial.println(F("CRC error!"));
                  break;
                }
                else {
                  if (hex_type == 1) {
                    _EOF = true;
                    break;
                  }
                  else {
                    hex_count=0;//next line
                    hex_crc = 0;
                    break;
                  }
                }
              }
              if (hex_count == (hex_len*2 + 9)) {
                dat = chr2hex(inChar);
                hex_count++;
                break;
              }
              if (hex_count < (hex_len*2 + 9)) {
                  if (hex_type == 0) {                  
                    if ((hex_count %2) != 0) {
                      dat = chr2hex(inChar);
                    }
                    else {
                      dat = chr2hex(inChar) + dat*16;
                      _AB = adr;
                      _DB = dat;
                      _WRMEM();
                      hex_bytes++;            
                      adr++;        
                      hex_crc = hex_crc + dat;
                    }
                    hex_count++; 
                  }
                  else {
                    error = true;
                    Serial.println(F("Type error"));
                  }
                  break;   
              }
          }
          Serial.write(0x06);//ACK
      }
      else {
        delay(10);
      }
    } while ((!_EOF) && (!error));
    if (!error) {
      Serial.println("");
      Serial.print(hex_bytes, DEC);
      Serial.println(F(" byte(s) were successfully received"));  
    }
    } 
           

    //TXXXX - load text file to memory (0x1A - EOF)
    if (mon_buffer[0]=='T') {  
    if (hexcheck(1,4)) {
    Serial.println(F("Ready to receive the text file..."));  
    _EOF = false;
    adr = kbd2word(1);
    count=0;
     do {
      inChar = '\0';
      if (con_ready()) {
        inChar = con_read();       
        count++;
        dat = uint8_t(inChar);
        _AB = adr;
        _DB = dat;
        _WRMEM();
        adr++;
        if (dat==0x1A) {
          //EOF
          _EOF = true;
        }
        Serial.write(0x06);//ACK send
      }
      else {
        delay(10);
      }
     } while (!_EOF);   
     Serial.println("");
     Serial.print(count, DEC);
     Serial.println(F(" byte(s) were successfully received"));  
     goto MON_END;
     }
        else {
          goto MON_INVALID;
      }
    } 

    //BXXXX - load binary file to memory
    if (mon_buffer[0]=='B') {  
    if (hexcheck(1,4)) {
    adr = kbd2word(1);
    tmp_word = adr;
    con_flush();
    Serial.println(F("Ready to receive the binary file..."));
    Serial.print(F("Address: "));
    Serial.print(adr, HEX);  
    _EOF = false;
    while (!con_ready()) {
      delay(10);
    }
    inChar = con_read();    
    len=uint8_t(inChar);
    while (!con_ready()) {
      delay(10);
    }
    inChar = con_read();    
    len=uint8_t(inChar)*256+len; 
    count=0;
    do {
      inChar = '\0';
      if (con_ready()) {
        inChar = con_read();       
        dat = uint8_t(inChar);
        _AB = adr;
        _DB = dat;
        _WRMEM();
        adr++;
        count++;
        if (count==len) {
          //EOF
          _EOF = true;
        }
        Serial.write(0x06);//ACK send
      }
      else {
        delay(10);
      }
     } while (!_EOF);        
     //crc receive
     while (!con_ready()) {
      delay(10);
    }
     inChar = con_read();    
     crc=uint8_t(inChar);
     //crc calculation
     tmp_byte=0;
     adr=tmp_word;
     for(count=0;count<len;count++) {
      _AB = adr;
      _RDMEM();
      tmp_byte=tmp_byte+_DB;
      adr++;
     }
     Serial.println("");
     if (crc==tmp_byte) {
      Serial.print(len, DEC);
      Serial.println(F(" byte(s) were successfully received")); 
     }
     else {
      Serial.println(F("CRC Error"));
     } 
     goto MON_END;
     }
        else {
          goto MON_INVALID;
      }
    }

    //M - Debug Mode on/off
    if (mon_buffer[0]=='W') {
      DEBUG = !DEBUG;
      if (DEBUG) {
        Serial.println(F("Debug ON"));
      }
      else {
        Serial.println(F("Debug OFF"));
     }
     goto MON_END;
    }

    //QXXXX - set breakpoint
    if (mon_buffer[0]=='Q') {
      if (hexcheck(1,4)) {
      byte res;
      adr = kbd2word(1);
      if (breakpoint == adr) {
        //breakpoint off
        Serial.print(F("Breakpoint disabled"));
      }
      else {
        //breakpoint on
        breakpoint = adr;
        Serial.print(F("Breakpoint: "));
        Serial.println(breakpoint, HEX); 
      }
      goto MON_END;
      }
        else {
          goto MON_INVALID;
      }
    }

    //GXXXX - run
    if (mon_buffer[0]=='G') {
      if (hexcheck(1,4)) {
      adr = kbd2word(1);
      clrarea();
      call(adr);
      Serial.println(F("O.K."));
      goto MON_END;
      }
        else {
          goto MON_INVALID;
      }
    }    

    //disk operations

    //S - sector print
    if (mon_buffer[0]=='S') {
      uint16_t i;
      uint8_t res;
      adr = kbd2word(2);
      switch (mon_buffer[1]) {
             case 'A'...char(uint8_t('A')+FDD_NUM-1): adr = adr + SD_FDD_OFFSET[uint8_t(mon_buffer[1]) - uint8_t('A')];
             break;
      }
      res = card.readBlock(adr, _dsk_buffer, 0);
      Serial.println(res, DEC);
      for (i = 0; i < SD_BLK_SIZE; i++) {
        Serial.print(_dsk_buffer[i], HEX);
        Serial.print(' ');
      }
      Serial.println(F("O.K."));
      goto MON_END;
    }

    //X - format disk
    if (mon_buffer[0]=='X') {
      uint8_t driveno;
      uint32_t start;
      uint8_t res;
      driveno = 0xFF;
      switch (mon_buffer[1]) {
        case 'A'...char(uint8_t('A')+FDD_NUM-1): driveno = (uint8_t(mon_buffer[1]) - uint8_t('A'));
             break;
      }
      if (driveno == 0xFF) {
        color(1);
        Serial.println(F("Invalid disk!"));
        color(9);
      }
      else {
        Serial.println(F("Format disk"));
        Serial.println("");
        //format
        start = SD_FDD_OFFSET[driveno];
        for (uint32_t i = 0; i<SD_BLK_SIZE; i++) {
          _dsk_buffer[i] = CPM_EMPTY;
        }
        for (uint32_t i = 0; i<DISK_SIZE*TRACK_SIZE; i++) {
          Serial.print('\r');
          Serial.print(F("SECTOR "));
          Serial.print(i,DEC);
          res = card.writeBlock(i+start, _dsk_buffer);
        }
        Serial.println("");
      }
      Serial.println(F("O.K."));
      goto MON_END;
    }

    //C - load CP/M
    if (mon_buffer[0]=='C') {
      DEBUG = false;//debug off 
      clrscr();//clear screen
      CPM_logo = true;
      while (!_IPL()) {};//initial loader
      CPM_logo = false;
      BIOS_INT = true;//BIOS intercept enabled
      MON = false;
      call(_BIOS);//JMP TO BIOS
      MON = true;
      DEBUG = true;//debug on
      goto MON_END;
    }

    //R - reset
    if (mon_buffer[0]=='R') {
      sys_reset();
    }

    //M - Memory  tesr
    if (mon_buffer[0]=='M') {
      uint32_t pass_cnt=0;
      boolean go=true;
      uint32_t temp;
      con_flush();
      clrscr();//clear screen
      Serial.println(F("RAM TEST..."));
      Serial.println(F("PRESS ANY KEY TO BREAK"));
      do {      
        xy(3,0);
        clrlin();
        temp = mem_test(true);
        if (temp == 0xFFFFF) {
          go = false;//break
        }
        else {
          if (RAM_AVAIL>temp) {
            pass_cnt++;
            RAM_AVAIL = temp;
          }
          pass_cnt++;
          Serial.println("");
          clrlin();
          Serial.print(F("PASS "));
          Serial.print(pass_cnt, DEC);
          Serial.print(F(" RAM: "));
          Serial.print(RAM_AVAIL, DEC);
          Serial.print(F(" BYTE(S)"));
        }
      } while (go);
     goto MON_END;
    }

    //EEPROM settings reset
    if (mon_buffer[0]=='E') {
       EEPROM_init();
       Serial.println(F("O.K."));
       goto MON_END;
    }

    //Z - insert floppy in drive
    //ZXYY  X - drives - A, B, C, D    Y - disks - 00..99
     if (mon_buffer[0]=='Z') {
      uint8_t driveno;
      uint8_t diskno;
      uint32_t start;
      uint8_t res;
      diskno = 0xFF;//disk number
      driveno = 0xFF;//drive number
      switch (mon_buffer[1]) {
        case 'A'...char(uint8_t('A')+FDD_NUM-1): driveno = (uint8_t(mon_buffer[1]) - uint8_t('A'));
             break;
      }
      if (driveno == 0xFF) {
        color(1);
        Serial.println(F("INVALID DRIVE!"));
        color(9);
      }
      else {
            //disk number input
            switch (mon_buffer[2]) {
              case '0'...'9': diskno = (uint8_t(mon_buffer[2]) - uint8_t('0'))*10;
                    break;
              default: diskno = 0xFF;
                    break;
            }
            switch (mon_buffer[3]) {
              case '0'...'9': diskno = uint8_t(mon_buffer[3]) - uint8_t('0') + diskno;
                    break;
              default: diskno = 0xFF;
            }
            if (diskno == 0xFF) {
              color(1);
              Serial.println(F("INVALID DISK!"));
              color(9);
            }
            else {
               //insert disk in drive
               Serial.print(F("INSERT DISK "));
               Serial.print(diskno, DEC);
               Serial.print(F(" IN DRIVE "));
               Serial.println(mon_buffer[1]);
               //Serial.println(SD_DISKS_OFFSET + diskno*SD_DISK_SIZE, HEX);
               SD_FDD_OFFSET[driveno] =  SD_DISKS_OFFSET + diskno*SD_DISK_SIZE;
               //save diskno in EEPROM
               //EEPROM cells
               //0xFE - 0x55
               //0xFF - 0xAA
               //0x02 - drive A
               //0x03 - drive B
               //0x04 - drive C
               //0x05 - drive D
               //check EEPROM O.K.
               //write signature to EEPROM
               //0xFE - 0x55
               if ((EEPROM.read(0xFE) != 0x55) || (EEPROM.read(0xFF) != 0xAA)) {
                //EEPROM init
                EEPROM_init();
               }
                if (EEPROM.read(0x00+driveno) != diskno) {
                  EEPROM.write(0x00+driveno, diskno);  
               }
            }
      }
      goto MON_END;
     }

     //Altair/IMSAI sense switch (input port 0xFF)
     if (mon_buffer[0]=='K') {
      if (hexcheck(1,2)) {
        SENSE_SW = kbd2byte(1);
        if ((EEPROM.read(0xFE) != 0x55) || (EEPROM.read(0xFF) != 0xAA)) {
                EEPROM_init();
        }
        EEPROM.write(EEPROM_SENSE_SW, SENSE_SW);
        Serial.println(F("O.K."));
        goto MON_END;
      }
      else {
        goto MON_INVALID;
      }
    }

MON_INVALID:
  Serial.println(F("???"));//invalid command
MON_END:


    
