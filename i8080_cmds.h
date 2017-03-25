/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/

   //dest = (cmd & ~__RP) >> 4;
   //reg = cmd & ~__SSS;
   //reg = (cmd & ~__DDD) >> 3;
   //rp = (cmd & ~__RP) >> 4;
  
  switch (cmd) {

    //00
    case 0x00: 
      _I8080_NOP();
      break;
    case 0x01: 
      _I8080_LXI(_RP_BC);
      break;
    case 0x02: 
      _I8080_STAX(_RP_BC);
      break;
    case 0x03: 
      _I8080_INX(_Reg_B);
      break;
    case 0x04: 
      _I8080_INR(_Reg_B);
      break;
    case 0x05: 
      _I8080_DCR(_Reg_B);
      break;
    case 0x06: 
      _I8080_MVI(_Reg_B);
      break;
    case 0x07: 
      _I8080_RLC();
      break;
    case 0x08: 
      _I8080_();//*NOP
      break;
    case 0x09: 
      _I8080_DAD(_RP_BC);
      break;
    case 0x0A: 
      _I8080_LDAX(_RP_BC);
      break;
    case 0x0B: 
      _I8080_DCX(_Reg_B);
      break;
    case 0x0C: 
      _I8080_INR(_Reg_C);
      break;
    case 0x0D: 
      _I8080_DCR(_Reg_C);
      break;
    case 0x0E: 
      _I8080_MVI(_Reg_C);
      break;
    case 0x0F: 
      _I8080_RRC();
      break;

    //10
    case 0x10: 
      _I8080_();//*NOP
      break;
    case 0x11: 
      _I8080_LXI(_RP_DE);
      break;
    case 0x12: 
      _I8080_STAX(_RP_DE);
      break;
    case 0x13: 
      _I8080_INX(_RP_DE);
      break;
    case 0x14: 
      _I8080_INR(_Reg_D);
      break;
    case 0x15: 
      _I8080_DCR(_Reg_D);
      break;
    case 0x16: 
      _I8080_MVI(_Reg_D);
      break;
    case 0x17: 
      _I8080_RAL();
      break;
    case 0x18: 
      _I8080_();//*NOP
      break;
    case 0x19: 
      _I8080_DAD(_RP_DE);
      break;
    case 0x1A: 
      _I8080_LDAX(_RP_DE);
      break;
    case 0x1B: 
      _I8080_DCX(_RP_DE);
      break;
    case 0x1C: 
      _I8080_INR(_Reg_E);
      break;
    case 0x1D: 
      _I8080_DCR(_Reg_E);
      break;
    case 0x1E: 
      _I8080_MVI(_Reg_E);
      break;
    case 0x1F: 
      _I8080_RAR();
      break;

    //20
    case 0x20: 
      _I8080_();//*NOP
      break;
    case 0x21: 
      _I8080_LXI(_RP_HL);
      break;
    case 0x22: 
      _I8080_SHLD();
      break;
    case 0x23: 
      _I8080_INX(_RP_HL);
      break;
    case 0x24: 
      _I8080_INR(_Reg_H);
      break;
    case 0x25: 
      _I8080_DCR(_Reg_H);
      break;
    case 0x26: 
      _I8080_MVI(_Reg_H);
      break;
    case 0x27: 
      _I8080_DAA();
      break;
    case 0x28: 
      _I8080_();//*NOP
      break;
    case 0x29: 
      _I8080_DAD(_RP_HL);
      break;
    case 0x2A: 
      _I8080_LHLD();
      break;
    case 0x2B: 
      _I8080_DCX(_RP_HL);
      break;
    case 0x2C: 
      _I8080_INR(_Reg_L);
      break;
    case 0x2D: 
      _I8080_DCR(_Reg_L);
      break;
    case 0x2E: 
      _I8080_MVI(_Reg_L);
      break;
    case 0x2F: 
      _I8080_CMA();
      break;


    //30
    case 0x30: 
      _I8080_();//*NOP
      break;
    case 0x31: 
      _I8080_LXI(_RP_SP);
      break;
    case 0x32: 
      _I8080_STA();
      break;
    case 0x33: 
      _I8080_INX(_RP_SP);
      break;
    case 0x34: 
      _I8080_INR(_Reg_M);
      break;
    case 0x35: 
      _I8080_DCR(_Reg_M);
      break;
    case 0x36: 
      _I8080_MVI(_Reg_M);
      break;
    case 0x37: 
      _I8080_STC();
      break;
    case 0x38: 
      _I8080_();//*NOP
      break;
    case 0x39: 
      _I8080_DAD(_RP_SP);
      break;
    case 0x3A: 
      _I8080_LDA();
      break;
    case 0x3B: 
      _I8080_DCX(_RP_SP);
      break;
    case 0x3C: 
      _I8080_INR(_Reg_A);
      break;
    case 0x3D: 
      _I8080_DCR(_Reg_A);
      break;
    case 0x3E: 
      _I8080_MVI(_Reg_A);
      break;
    case 0x3F: 
      _I8080_CMC();
      break;


    //40 MOV dest src
    case 0x40: 
      _I8080_MOV(_Reg_B, _Reg_B);
      break;
    case 0x41: 
      _I8080_MOV(_Reg_B, _Reg_C);
      break;
    case 0x42: 
      _I8080_MOV(_Reg_B, _Reg_D);
      break;
    case 0x43: 
      _I8080_MOV(_Reg_B, _Reg_E);
      break;
    case 0x44: 
      _I8080_MOV(_Reg_B, _Reg_H);
      break;
    case 0x45: 
      _I8080_MOV(_Reg_B, _Reg_L);
      break;
    case 0x46: 
      _I8080_MOV(_Reg_B, _Reg_M);
      break;
    case 0x47: 
      _I8080_MOV(_Reg_B, _Reg_A);
      break;
    case 0x48: 
      _I8080_MOV(_Reg_C, _Reg_B);
      break;
    case 0x49: 
      _I8080_MOV(_Reg_C, _Reg_C);
      break;
    case 0x4A: 
      _I8080_MOV(_Reg_C, _Reg_D);
      break;
    case 0x4B: 
      _I8080_MOV(_Reg_C, _Reg_E);
      break;
    case 0x4C: 
      _I8080_MOV(_Reg_C, _Reg_H);
      break;
    case 0x4D: 
      _I8080_MOV(_Reg_C, _Reg_L);
      break;
    case 0x4E: 
      _I8080_MOV(_Reg_C, _Reg_M);
      break;
    case 0x4F: 
      _I8080_MOV(_Reg_C, _Reg_A);
      break;

    //50
    case 0x50: 
      _I8080_MOV(_Reg_D, _Reg_B);
      break;
    case 0x51: 
      _I8080_MOV(_Reg_D, _Reg_C);
      break;
    case 0x52: 
      _I8080_MOV(_Reg_D, _Reg_D);
      break;
    case 0x53: 
      _I8080_MOV(_Reg_D, _Reg_E);
      break;
    case 0x54: 
      _I8080_MOV(_Reg_D, _Reg_H);
      break;
    case 0x55: 
      _I8080_MOV(_Reg_D, _Reg_L);
      break;
    case 0x56: 
      _I8080_MOV(_Reg_D, _Reg_M);
      break;
    case 0x57: 
      _I8080_MOV(_Reg_D, _Reg_A);
      break;
    case 0x58: 
      _I8080_MOV(_Reg_E, _Reg_B);
      break;
    case 0x59: 
      _I8080_MOV(_Reg_E, _Reg_C);
      break;
    case 0x5A: 
      _I8080_MOV(_Reg_E, _Reg_D);
      break;
    case 0x5B: 
      _I8080_MOV(_Reg_E, _Reg_E);
      break;
    case 0x5C: 
      _I8080_MOV(_Reg_E, _Reg_H);
      break;
    case 0x5D: 
      _I8080_MOV(_Reg_E, _Reg_L);
      break;
    case 0x5E: 
      _I8080_MOV(_Reg_E, _Reg_M);
      break;
    case 0x5F: 
      _I8080_MOV(_Reg_E, _Reg_A);
      break;

    //60
    case 0x60: 
      _I8080_MOV(_Reg_H, _Reg_B);
      break;
    case 0x61: 
      _I8080_MOV(_Reg_H, _Reg_C);
      break;
    case 0x62: 
      _I8080_MOV(_Reg_H, _Reg_D);
      break;
    case 0x63: 
      _I8080_MOV(_Reg_H, _Reg_E);
      break;
    case 0x64: 
      _I8080_MOV(_Reg_H, _Reg_H);
      break;
    case 0x65: 
      _I8080_MOV(_Reg_H, _Reg_L);
      break;
    case 0x66: 
      _I8080_MOV(_Reg_H, _Reg_M);
      break;
    case 0x67: 
      _I8080_MOV(_Reg_H, _Reg_A);
      break;
    case 0x68: 
      _I8080_MOV(_Reg_L, _Reg_B);
      break;
    case 0x69: 
      _I8080_MOV(_Reg_L, _Reg_C);
      break;
    case 0x6A: 
      _I8080_MOV(_Reg_L, _Reg_D);
      break;
    case 0x6B: 
      _I8080_MOV(_Reg_L, _Reg_E);
      break;
    case 0x6C: 
      _I8080_MOV(_Reg_L, _Reg_H);
      break;
    case 0x6D: 
      _I8080_MOV(_Reg_L, _Reg_L);
      break;
    case 0x6E: 
      _I8080_MOV(_Reg_L, _Reg_M);
      break;
    case 0x6F: 
      _I8080_MOV(_Reg_L, _Reg_A);
      break;

    //70
    case 0x70: 
      _I8080_MOV(_Reg_M, _Reg_B);
      break;
    case 0x71: 
      _I8080_MOV(_Reg_M, _Reg_C);
      break;
    case 0x72: 
      _I8080_MOV(_Reg_M, _Reg_D);
      break;
    case 0x73: 
      _I8080_MOV(_Reg_M, _Reg_E);
      break;
    case 0x74: 
      _I8080_MOV(_Reg_M, _Reg_H);
      break;
    case 0x75: 
      _I8080_MOV(_Reg_M, _Reg_L);
      break;
    case 0x76: 
      _I8080_HLT();
      break;
    case 0x77: 
      _I8080_MOV(_Reg_M, _Reg_A);
      break;
    case 0x78: 
      _I8080_MOV(_Reg_A, _Reg_B);
      break;
    case 0x79: 
      _I8080_MOV(_Reg_A, _Reg_C);
      break;
    case 0x7A: 
      _I8080_MOV(_Reg_A, _Reg_D);
      break;
    case 0x7B: 
      _I8080_MOV(_Reg_A, _Reg_E);
      break;
    case 0x7C: 
      _I8080_MOV(_Reg_A, _Reg_H);
      break;
    case 0x7D: 
      _I8080_MOV(_Reg_A, _Reg_L);
      break;
    case 0x7E: 
      _I8080_MOV(_Reg_A, _Reg_M);
      break;
    case 0x7F: 
      _I8080_MOV(_Reg_A, _Reg_A);
      break;


    //80
    case 0x80: 
      _I8080_ADD(_Reg_B);
      break;
    case 0x81: 
      _I8080_ADD(_Reg_C);
      break;
    case 0x82: 
      _I8080_ADD(_Reg_D);
      break;
    case 0x83: 
      _I8080_ADD(_Reg_E);
      break;
    case 0x84: 
      _I8080_ADD(_Reg_H);   
      break;
    case 0x85: 
      _I8080_ADD(_Reg_L);
      break;
    case 0x86: 
      _I8080_ADD(_Reg_M);
      break;
    case 0x87: 
      _I8080_ADD(_Reg_A);
      break;
    case 0x88: 
      _I8080_ADC(_Reg_B);
      break;
    case 0x89: 
      _I8080_ADC(_Reg_C);
      break;
    case 0x8A: 
      _I8080_ADC(_Reg_D);
      break;
    case 0x8B: 
      _I8080_ADC(_Reg_E);
      break;
    case 0x8C: 
      _I8080_ADC(_Reg_H);
      break;
    case 0x8D: 
      _I8080_ADC(_Reg_L);
      break;
    case 0x8E: 
      _I8080_ADC(_Reg_M);
      break;
    case 0x8F: 
      _I8080_ADC(_Reg_A);
      break;

    //90
    case 0x90: 
      _I8080_SUB(_Reg_B);
      break;
    case 0x91: 
      _I8080_SUB(_Reg_C);
      break;
    case 0x92: 
      _I8080_SUB(_Reg_D);
      break;
    case 0x93: 
      _I8080_SUB(_Reg_E);
      break;
    case 0x94: 
      _I8080_SUB(_Reg_H);
      break;
    case 0x95: 
      _I8080_SUB(_Reg_L);
      break;
    case 0x96: 
      _I8080_SUB(_Reg_M);
      break;
    case 0x97: 
      _I8080_SUB(_Reg_A);
      break;
    case 0x98: 
      _I8080_SBB(_Reg_B);
      break;
    case 0x99: 
      _I8080_SBB(_Reg_C);
      break;
    case 0x9A: 
      _I8080_SBB(_Reg_D);
      break;
    case 0x9B: 
      _I8080_SBB(_Reg_E);
      break;
    case 0x9C: 
      _I8080_SBB(_Reg_H);
      break;
    case 0x9D: 
      _I8080_SBB(_Reg_L);
      break;
    case 0x9E: 
      _I8080_SBB(_Reg_M);
      break;
    case 0x9F: 
      _I8080_SBB(_Reg_A);
      break;

    //A0
    case 0xA0: 
      _I8080_ANA(_Reg_B);
      break;
    case 0xA1: 
      _I8080_ANA(_Reg_C);
      break;
    case 0xA2: 
      _I8080_ANA(_Reg_D);
      break;
    case 0xA3: 
      _I8080_ANA(_Reg_E);
      break;
    case 0xA4: 
      _I8080_ANA(_Reg_H);
      break;
    case 0xA5: 
      _I8080_ANA(_Reg_L);
      break;
    case 0xA6: 
      _I8080_ANA(_Reg_M);
      break;
    case 0xA7: 
      _I8080_ANA(_Reg_A);
      break;
    case 0xA8: 
      _I8080_XRA(_Reg_B);
      break;
    case 0xA9: 
      _I8080_XRA(_Reg_C);
      break;
    case 0xAA: 
      _I8080_XRA(_Reg_D);
      break;
    case 0xAB: 
      _I8080_XRA(_Reg_E);
      break;
    case 0xAC: 
      _I8080_XRA(_Reg_H);
      break;
    case 0xAD: 
      _I8080_XRA(_Reg_L);
      break;
    case 0xAE: 
      _I8080_XRA(_Reg_M);
      break;
    case 0xAF: 
      _I8080_XRA(_Reg_A);
      break;

    //B0
    case 0xB0: 
      _I8080_ORA(_Reg_B);
      break;
    case 0xB1: 
      _I8080_ORA(_Reg_C);
      break;
    case 0xB2: 
      _I8080_ORA(_Reg_D);
      break;
    case 0xB3: 
      _I8080_ORA(_Reg_E);
      break;
    case 0xB4: 
      _I8080_ORA(_Reg_H);
      break;
    case 0xB5: 
      _I8080_ORA(_Reg_L);
      break;
    case 0xB6: 
      _I8080_ORA(_Reg_M);
      break;
    case 0xB7: 
      _I8080_ORA(_Reg_A);
      break;
    case 0xB8: 
      _I8080_CMP(_Reg_B);
      break;
    case 0xB9: 
      _I8080_CMP(_Reg_C);
      break;
    case 0xBA: 
      _I8080_CMP(_Reg_D);
      break;
    case 0xBB: 
      _I8080_CMP(_Reg_E);
      break;
    case 0xBC: 
      _I8080_CMP(_Reg_H);
      break;
    case 0xBD: 
      _I8080_CMP(_Reg_L);
      break;
    case 0xBE: 
      _I8080_CMP(_Reg_M);
      break;
    case 0xBF: 
      _I8080_CMP(_Reg_A);
      break;

    //C0
    case 0xC0: 
      _I8080_RNZ();
      break;
    case 0xC1: 
      _I8080_POP(_RP_BC);
      break;
    case 0xC2: 
      _I8080_JNZ();
      break;
    case 0xC3: 
      _I8080_JMP();
      break;
    case 0xC4: 
      _I8080_CNZ();
      break;
    case 0xC5: 
      _I8080_PUSH(_RP_BC);
      break;
    case 0xC6: 
      _I8080_ADI();
      break;
    case 0xC7: 
      _I8080_RST(0);
      break;
    case 0xC8: 
      _I8080_RZ();
      break;
    case 0xC9: 
      _I8080_RET();
      break;
    case 0xCA: 
      _I8080_JZ();
      break;
    case 0xCB: 
      _I8080_JMP();
      break;
    case 0xCC: 
      _I8080_CZ();
      break;
    case 0xCD: 
      _I8080_CALL();
      break;
    case 0xCE: 
      _I8080_ACI();
      break;
    case 0xCF: 
      _I8080_RST(1);
      break;

    //D0
    case 0xD0: 
      _I8080_RNC();
      break;
    case 0xD1: 
      _I8080_POP(_RP_DE);
      break;
    case 0xD2: 
      _I8080_JNC();
      break;
    case 0xD3: 
      _I8080_OUT();
      break;
    case 0xD4: 
      _I8080_CNC();
      break;
    case 0xD5: 
      _I8080_PUSH(_RP_DE);
      break;
    case 0xD6: 
      _I8080_SUI();
      break;
    case 0xD7: 
      _I8080_RST(2);
      break;
    case 0xD8: 
      _I8080_RC();
      break;
    case 0xD9: 
      _I8080_RET();
      break;
    case 0xDA: 
      _I8080_JC();
      break;
    case 0xDB: 
      _I8080_IN();
      break;
    case 0xDC: 
      _I8080_CC();
      break;
    case 0xDD: 
      _I8080_CALL();
      break;
    case 0xDE: 
      _I8080_SBI();
      break;
    case 0xDF: 
      _I8080_RST(3);
      break;

    //E0
    case 0xE0: 
      _I8080_RPO();
      break;
    case 0xE1: 
      _I8080_POP(_RP_HL);
      break;
    case 0xE2: 
      _I8080_JPO();
      break;
    case 0xE3: 
      _I8080_XTHL();
      break;
    case 0xE4: 
      _I8080_CPO();
      break;
    case 0xE5: 
      _I8080_PUSH(_RP_HL);
      break;
    case 0xE6: 
      _I8080_ANI();
      break;
    case 0xE7: 
      _I8080_RST(4);
      break;
    case 0xE8: 
      _I8080_RPE();
      break;
    case 0xE9: 
      _I8080_PCHL();
      break;
    case 0xEA: 
      _I8080_JPE();
      break;
    case 0xEB: 
      _I8080_XCHG();
      break;
    case 0xEC: 
      _I8080_CPE();
      break;
    case 0xED: 
      _I8080_CALL();
      break;
    case 0xEE: 
      _I8080_XRI();
      break;
    case 0xEF: 
      _I8080_RST(5);
      break;

    //F0
    case 0xF0: 
      _I8080_RP();
      break;
    case 0xF1: 
      _I8080_POP(_RP_AF);
      break;
    case 0xF2: 
      _I8080_JP();
      break;
    case 0xF3: 
      _I8080_DI();
      break;
    case 0xF4: 
      _I8080_CP();
      break;
    case 0xF5: 
      _I8080_PUSH(_RP_AF);
      break;
    case 0xF6: 
      _I8080_ORI();
      break;
    case 0xF7: 
      _I8080_RST(6);
      break;
    case 0xF8: 
      _I8080_RM();
      break;
    case 0xF9: 
      _I8080_SPHL();
      break;
    case 0xFA: 
      _I8080_JM();
      break;
    case 0xFB: 
      _I8080_EI();
      break;
    case 0xFC: 
      _I8080_CM();
      break;
    case 0xFD: 
      _I8080_CALL();
      break;
    case 0xFE: 
      _I8080_CPI();
      break;
    case 0xFF: 
      _I8080_RST(7);
      break;



  }
  
  
 


