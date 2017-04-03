/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/
  
  exe_flag = true;

  if (_PC ==  FBASE) {
      if (DEBUG) 
      {
      color(3);
      Serial.print(F("BDOS Fn:"));
      Serial.print(_Regs[_Reg_C], HEX);
      Serial.print(F(" DE="));
      Serial.print(_Regs[_Reg_D], HEX);
      Serial.println(_Regs[_Reg_E], HEX);
      color(9);
      }
  }

  if ((_PC>=_BIOS_LO) && (_PC<_BIOS_HI) && BIOS_INT) {
    if (DEBUG) 
    {
    color(4);
    Serial.print(F("BIOS Fn:"));
    Serial.print(_PC, HEX);
    Serial.print(F(" C="));
    Serial.println(_Regs[_Reg_C], HEX);
    color(9);
    }
    switch (_PC) {
      case _BIOS + 0U://BOOT
        _BIOS_BOOT();
        exe_flag = false;
        break;
      case _BIOS + 3U://WBOOT
        _BIOS_WBOOT();
        exe_flag = false;
        break;
      //ASCII 7 ‚ 0
      //CTRL-Z 0x1A
      case _BIOS + 6U://CONST
        _BIOS_CONST();
        exe_flag = false;
        break;
      case _BIOS + 9U://CONIN
        _BIOS_CONIN();
        exe_flag = false;
        break;
      case _BIOS + 0xcU://CONOUT
        _BIOS_CONOUT();
        exe_flag = false;
        break;
      case _BIOS + 0xfU://LIST
        _BIOS_LIST();
        exe_flag = false;
        break;
      case _BIOS + 0x12U://PUNCH
        _BIOS_PUNCH();
        exe_flag = false;
        break;
      case _BIOS + 0x15U://READER 
        _BIOS_READER();
        exe_flag = false;
        break;
      case _BIOS + 0x2dU://LISTST
        _BIOS_LISTST();
        exe_flag = false;
        break;
      case _BIOS + 0x18U://HOME
        _BIOS_HOME();
        exe_flag = false;
        break;
      case _BIOS + 0x1bU://SELDSK
        _BIOS_SELDSK();
        exe_flag = false;
        break;
      case _BIOS + 0x1eU://SETTRK
        _BIOS_SETTRK();
        exe_flag = false;
        break;
      case _BIOS + 0x21U://SETSEC
        _BIOS_SETSEC();
        exe_flag = false;
        break;
      case _BIOS + 0x24U://SETDMA
        _BIOS_SETDMA();
        exe_flag = false;
        break;
      case _BIOS + 0x27U://READ
        _BIOS_READ();
        exe_flag = false;
        break;
      case _BIOS + 0x2aU://WRITE
        _BIOS_WRITE();
        exe_flag = false;
        break;
      case _BIOS + 0x30U://SECTRAN
        _BIOS_SECTRAN();
        exe_flag = false;
        break;
      default:
        exe_flag = false;
        exitFlag = true;//BIOS error
        break;  
    }
    if (!exe_flag) {
      if (exitFlag) {
        break;
      }
      else {
        continue;
      }
    }
  }

