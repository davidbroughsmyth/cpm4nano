/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/

//TO DO
//command length check

    clrarea();//clear work area
    
    //LXXXXYY - load byte to memory
    if (KbdBuffer[0]=='L') {
      if (hexcheck(1,6)) {
        adr = kbd2word(1);
        dat = kbd2byte(5);
        _setMEM(adr,dat);
        Serial.println("O.K.");
        goto MON_END;
      }
      else {
        goto MON_INVALID;
      }
    }

    //DXXXX - dump byte from memory
    if (KbdBuffer[0]=='D') {
        if (hexcheck(1,4)) {
          adr = kbd2word(1);
          dat = _getMEM(adr);
          Serial.println(dat, HEX);
          Serial.println(F("O.K."));
          goto MON_END;
        }
        else {
          goto MON_INVALID;
      }
    }

    //OXXYY - send byte to port
    if (KbdBuffer[0]=='O') {
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
    if (KbdBuffer[0]=='I') {
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
    if (KbdBuffer[0]=='F') {  
    Serial.println(F("Ready to receive the HEX file..."));  
    error = false;
    _EOF = false;
    hex_crc = 0;
    hex_count = 0;
    hex_bytes = 0;
    hex_type = 0;
    do {
      inChar = '\0';
      if (Serial.available() > 0) {
        inChar = Serial.read();       
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
                      _setMEM(adr, dat);
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
    } while ((!_EOF) && (!error));
    if (!error) {
      Serial.println("");
      Serial.print(hex_bytes, DEC);
      Serial.println(F(" byte(s) were successfully received"));  
    }
    } 
           

    //TXXXX - load text file to memory (0x1A - EOF)
    if (KbdBuffer[0]=='T') {  
    if (hexcheck(1,4)) {
    Serial.println(F("Ready to receive the text file..."));  
    _EOF = false;
    adr = kbd2word(1);
    count=0;
     do {
      inChar = '\0';
      if (Serial.available() > 0) {
        inChar = Serial.read();       
        count++;
        dat = uint8_t(inChar);
        _setMEM(adr, dat);
        adr++;
        if (dat==0x1A) {
          //EOF
          _EOF = true;
        }
        Serial.write(0x06);//ACK send
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
    if (KbdBuffer[0]=='B') {  
    if (hexcheck(1,4)) {
    adr = kbd2word(1);
    tmp_word = adr;
    Serial.println(F("Ready to receive the binary file..."));
    Serial.print(F("Address: "));
    Serial.print(adr, HEX);  
    _EOF = false;
    while (Serial.available() == 0) {
    }
    inChar = Serial.read();    
    len=uint8_t(inChar);
    while (Serial.available() == 0) {
    }
    inChar = Serial.read();    
    len=uint8_t(inChar)*256+len; 
    count=0;
    do {
      inChar = '\0';
      if (Serial.available() > 0) {
        inChar = Serial.read();       
        dat = uint8_t(inChar);
        _setMEM(adr, dat);
        adr++;
        count++;
        if (count==len) {
          //EOF
          _EOF = true;
        }
        Serial.write(0x06);//ACK send
      }
     } while (!_EOF);        
     //crc receive
     while (Serial.available() == 0) {
     }
     inChar = Serial.read();    
     crc=uint8_t(inChar);
     //crc calculation
     tmp_byte=0;
     adr=tmp_word;
     for(count=0;count<len;count++) {
      tmp_byte=tmp_byte+_getMEM(adr);
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
    if (KbdBuffer[0]=='M') {
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
    if (KbdBuffer[0]=='Q') {
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

    //Z - Z80 Emulation On/Off
    if (KbdBuffer[0]=='Z') {
      Z80 = !Z80;
      if (Z80) {
        Serial.println(F("Z80 Emulation ON"));
      }
      else {
        Serial.println(F("Z80 Emulation OFF"));
     }
     goto MON_END;
    }

    //GXXXX - run
    if (KbdBuffer[0]=='G') {
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
    if (KbdBuffer[0]=='S') {
      uint16_t i;
      byte res;
      adr = kbd2word(2);
      switch (KbdBuffer[1]) {
        case 'A': adr = adr + SD_FDD_A_OFFSET;
                  break;
        case 'B': adr = adr + SD_FDD_B_OFFSET;
                  break;
        case 'C': adr = adr + SD_FDD_C_OFFSET;
                  break;
        case 'D': adr = adr + SD_FDD_D_OFFSET;
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
    if (KbdBuffer[0]=='X') {
      uint8_t diskno;
      uint32_t start;
      diskno = 0xFF;
      switch (KbdBuffer[1]) {
        case 'A': diskno = 0;
             break;
      }
      if (diskno == 0xFF) {
        color(1);
        Serial.println(F("Invalid disk!"));
        color(9);
      }
      else {
        Serial.println(F("Format disk"));
        Serial.println("");
        //format
        start = SD_FDD_A_OFFSET; //+ diskno*0x400;
        for (uint32_t i = 0; i<SD_BLK_SIZE; i++) {
          _buffer[i] = CPM_EMPTY;
        }
        for (uint32_t i = 0; i<DISK_SIZE*TRACK_SIZE; i++) {
          Serial.print('\r');
          Serial.print(F("Sector "));
          Serial.print(i,DEC);
          writeSD(i+start);
        }
        Serial.println("");
      }
      Serial.println(F("O.K."));
      goto MON_END;
    }

    //C - load CP/M
    if (KbdBuffer[0]=='C') {
      DEBUG = false;//debug off 
      clrscr();//clear screen
      while (!_IPL()) {};//initial loader
      BIOS_INT = true;//BIOS intercept enabled
      call(_BIOS);//JMP TO BIOS
      DEBUG = true;//debug on
      goto MON_END;
    }

MON_INVALID:
  Serial.println(F("???"));//invalid command
MON_END:


    
