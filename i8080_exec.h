/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0
    Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab
    Email:    support@foxylab.com
    Website:  https://acdc.foxylab.com
*/

//TO DO
//INR  DCR - AC


#define _Reg_B B000
#define _Reg_C B001
#define _Reg_D B010
#define _Reg_E B011
#define _Reg_H B100
#define _Reg_L B101
#define _Reg_M B110
#define _Reg_A B111

#define _RP_BC B00
#define _RP_DE B01
#define _RP_HL B10
#define _RP_SP B11
#define _RP_AF B11

uint8_t _Regs[8];
/*
  0 - B
  1 - C
  2 - D
  3 - E
  4 - H
  5 - L
  6 - M *Flags
  7 - A
*/

uint8_t SZP_RESET = B00111011;
uint8_t ZP_RESET = B10111011;
#define _setFlags_S(b) bitWrite(_Regs[_Reg_M],7,b)
#define _setFlags_Z(b) bitWrite(_Regs[_Reg_M],6,b)
#define _setFlags_A(b) bitWrite(_Regs[_Reg_M],4,b)
#define _setFlags_P(b) bitWrite(_Regs[_Reg_M],2,b)
#define _setFlags_C(b) bitWrite(_Regs[_Reg_M],0,b)

#define _getFlags_S() bitRead(_Regs[_Reg_M],7)
#define _getFlags_Z() bitRead(_Regs[_Reg_M],6)
#define _getFlags_A() bitRead(_Regs[_Reg_M],4)
#define _getFlags_P() bitRead(_Regs[_Reg_M],2)
#define _getFlags_C() bitRead(_Regs[_Reg_M],0)

uint16_t _PC; //program counter
uint16_t  _SP; //stack pointer

bool INTE;

uint16_t sp2a16() {
  uint16_t a16;
  a16 = _getMEM(_SP);
  _SP++;
  a16 = a16 + 256 * _getMEM(_SP);
  _SP++;
  return a16;
}

uint16_t pc2a16() {
  uint16_t a16;
  _PC++;
  a16 = _getMEM(_PC);
  _PC++;
  a16 = a16 + 256 * _getMEM(_PC);
  return a16;
}

void pc2sp() {
  _SP--;
  _SP--;
  _setMEM(_SP, lowByte(_PC));
  _setMEM(_SP + 1, highByte(_PC));
}

uint8_t in_port(uint8_t port) {
  uint8_t dat;
  boolean readyFlag=false;
  dat = 0x00;
  switch (port) {
    //SIO-A/SSM
    case SIOA_CON_PORT_STATUS:
      //SIO-A
      //bit 1 - ready to out
      //bit 5 - ready to in
      //SSM 
      //bit 7 - ready to out
      //bit 0 - ready to in
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
      //dat = _getMEM(_FDD_TRACK);
      break;
    case FDD_PORT_SEC:
      //sector
      dat = FDD_REG_SEC;
      //dat = _getMEM(_FDD_SECTOR);
      break;
    case FDD_PORT_DRV:
      //drive select
      dat = FDD_REG_DRV;
      //dat = _getMEM(_DISKNO);
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
  return dat;
}


void out_port(uint8_t port, uint8_t dat) {
  uint16_t i;
  uint8_t res;
  uint32_t blk;
  switch (port) {
    //console ports
    //SIO-A/SSM
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
        switch (FDD_REG_DRV) {
          case 0:
            blk = blk +  SD_FDD_A_OFFSET;
            break;
          case 1:
            blk = blk +  SD_FDD_B_OFFSET;
            break;
          case 2:
            blk = blk +  SD_FDD_C_OFFSET;
            break;
          case 3:
            blk = blk +  SD_FDD_D_OFFSET;
            break;
        }
        //Serial.print("Read block ");
        //Serial.println(blk, HEX);
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
        //blk = _getMEM(_FDD_SECTOR)-1;
        blk = FDD_REG_SEC - 1L;
        blk = blk + FDD_REG_TRK * TRACK_SIZE;
        switch (FDD_REG_DRV) {
          case 0:
            blk = blk +  SD_FDD_A_OFFSET;
            break;
          case 1:
            blk = blk +  SD_FDD_B_OFFSET;
            break;
          case 2:
            blk = blk +  SD_FDD_C_OFFSET;
            break;
          case 3:
            blk = blk +  SD_FDD_D_OFFSET;
            break;
        }
        for (i = 0 ; i < SD_BLK_SIZE ; i++) {
          _dsk_buffer[i] = _getMEM(FDD_REG_DMA + i);
        }
        for (i = 0; i < SD_BLK_SIZE; i++) {
          _buffer[i] = _dsk_buffer[i];
        }
        //Serial.print("Write block ");
        //Serial.println(blk, HEX);
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
      //_setMEM(_FDD_TRACK, dat);
      break;
    case FDD_PORT_SEC:
      //sector
      //_setMEM(_FDD_SECTOR, dat);
      FDD_REG_SEC = dat;
      break;
    case FDD_PORT_DRV:
      //drive select
      FDD_REG_DRV = dat;
      //_setMEM(_DISKNO, dat);
      break;
    case FDD_PORT_DMA_ADDR_LO:
      FDD_REG_DMA = FDD_REG_DMA & 0xFF00;
      FDD_REG_DMA = FDD_REG_DMA | dat;
      //_setMEM(_FDD_DMA_ADDRESS, dat);
      break;
    case FDD_PORT_DMA_ADDR_HI:
      FDD_REG_DMA = FDD_REG_DMA & 0x00FF;
      FDD_REG_DMA = FDD_REG_DMA + dat*256;
      //_setMEM(_FDD_DMA_ADDRESS + 1, dat);
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

void _I8080_() {
  _PC++;
}

void _I8080_HLT() {
  exitFlag = true;
}

//NOP
void _I8080_NOP() {
  _PC++;
}

void _I8080_MOV(uint8_t dest, uint8_t src) {
  if (dest == _Reg_M)
  {
    _setMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]), _Regs[src]);
  }
  else
  {
    if (src == _Reg_M)
    {
      _Regs[dest] = _getMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]));
    }
    else
    {
      _Regs[dest] = _Regs[src];
    }
  }
  _PC++;
}

//MVI
void _I8080_MVI(uint8_t dest) {
  uint8_t d8;
  _PC++;
  d8 = _getMEM(_PC);
  if (dest != _Reg_M)
  {
    _Regs[dest] = d8;
  }
  else
  {
    _setMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]), d8);
  }
  _PC++;
}

//LXI
void _I8080_LXI(uint8_t rp) {
  uint16_t d16;
  _PC++;
  d16 = _getMEM(_PC);
  _PC++;
  d16 = d16 + 256 * _getMEM(_PC);
  switch (rp) {
    case _RP_BC:
      _Regs[_Reg_B] = highByte(d16);
      _Regs[_Reg_C] = lowByte(d16);
      break;
    case _RP_DE:
      _Regs[_Reg_D] = highByte(d16);
      _Regs[_Reg_E] = lowByte(d16);
      break;
    case _RP_HL:
      _Regs[_Reg_H] = highByte(d16);
      _Regs[_Reg_L] = lowByte(d16);
      break;
    case _RP_SP:
      _SP = d16;
      break;
  }
  _PC++;
}

//LDA
void _I8080_LDA() {
  _Regs[_Reg_A] = _getMEM(pc2a16());
  _PC++;
}

//STA
void _I8080_STA() {
  _setMEM(pc2a16(), _Regs[_Reg_A]);
  _PC++;
}

//LHLD
void _I8080_LHLD()
{
  uint16_t a16;
  a16 = pc2a16();
  _Regs[_Reg_L] = _getMEM(a16);
  a16++;
  _Regs[_Reg_H] = _getMEM(a16);
  _PC++;
}

//SHLD
void _I8080_SHLD() {
  uint16_t a16;
  a16 = pc2a16();
  _setMEM(a16, _Regs[_Reg_L]);
  a16++;
  _setMEM(a16, _Regs[_Reg_H]);
  _PC++;
}

//LDAX
void _I8080_LDAX(uint8_t rp) {
  uint16_t a16;
  switch (rp) {
    case _RP_BC:
      a16 = _Regs[_Reg_C] + 256 * _Regs[_Reg_B];
      break;
    case _RP_DE:
      a16 = _Regs[_Reg_E] + 256 * _Regs[_Reg_D];
      break;
  }
  _Regs[_Reg_A] = _getMEM(a16);
  _PC++;
}

//STAX
void _I8080_STAX(uint8_t rp) {
  uint16_t a16;
  switch (rp) {
    case _RP_BC:
      a16 = _Regs[_Reg_C] + 256 * _Regs[_Reg_B];
      break;
    case _RP_DE:
      a16 = _Regs[_Reg_E] + 256 * _Regs[_Reg_D];
      break;
  }
  _setMEM(a16, _Regs[_Reg_A]);
  _PC++;
}

//XCHG
void _I8080_XCHG() {
  uint8_t d8;
  d8 = _Regs[_Reg_D];
  _Regs[_Reg_D] = _Regs[_Reg_H];
  _Regs[_Reg_H] = d8;
  d8 = _Regs[_Reg_E];
  _Regs[_Reg_E] = _Regs[_Reg_L];
  _Regs[_Reg_L] = d8;
  _PC++;
}

//ADD
void _I8080_ADD(uint8_t reg) {
  uint16_t d16;
  uint8_t d8;
  if (reg != _Reg_M) {
    d8 = _Regs[reg];
  }
  else {
    d8 = _getMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]));
  }
  if ((_Regs[_Reg_A] & B1111) + (d8 & B1111) > B1111) {
    _setFlags_A(1);
  }
  else {
    _setFlags_A(0);
  }
  d16 = _Regs[_Reg_A] + d8;
  d8 = lowByte(d16);
  if (d16 > 0xFF) {
    _setFlags_C(1);
  }
  else {
    _setFlags_C(0);
  }
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  _Regs[_Reg_A] = d8;
  _PC++;
}

//ADI
void _I8080_ADI() {
  uint16_t d16;
  uint8_t d8;
  _PC++;
  d8 = _getMEM(_PC);
  if ((_Regs[_Reg_A] & B1111) + (d8 & B1111) > B1111) {
    _setFlags_A(1);
  }
  else {
    _setFlags_A(0);
  }
  d16 = _Regs[_Reg_A] + d8;
  d8 = lowByte(d16);
  if (d16 > 0xFF) {
    _setFlags_C(1);
  }
  else {
    _setFlags_C(0);
  }
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  _Regs[_Reg_A] = d8;
  _PC++;
}

//ADC
void _I8080_ADC(uint8_t reg) {
  uint16_t d16;
  uint8_t d8;
  if (reg != _Reg_M) {
    d8 = _Regs[reg];
  }
  else {
    d8 = _getMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]));
  }
  if ((_Regs[_Reg_A] & B1111) + (d8 & B1111) > B1111) {
    _setFlags_A(1);
  }
  else {
    _setFlags_A(0);
  }
  d16 = _Regs[_Reg_A] + d8;
  if (_getFlags_C() != 0) //+ C
  {
    d16++;
  }
  d8 = lowByte(d16);
  if ((d16 & 0x0100)!=0) {
    _setFlags_C(1);
  }
  else {
    _setFlags_C(0);
  }
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  _Regs[_Reg_A] = d8;
  _PC++;
}

//ACI
void _I8080_ACI() {
  uint16_t d16;
  uint8_t d8;
  _PC++;
  d8 = _getMEM(_PC);
  if ((_Regs[_Reg_A] & B1111) + (d8 & B1111) > B1111) {
    _setFlags_A(1);
  }
  else {
    _setFlags_A(0);
  }
  d16 = _Regs[_Reg_A] + d8;
  if ((_Regs[_Reg_M] & 0x1) != 0)
  {
    d16 = d16 + 1;
  }
  d8 = lowByte(d16);
  if (d16 > 0xFF) {
    _setFlags_C(1);
  }
  else {
    _setFlags_C(0);
  }
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  _Regs[_Reg_A] = d8;
  _PC++;
}

//SUB
void _I8080_SUB(uint8_t reg) {
  uint16_t d16;
  uint8_t d8;
  if (reg != _Reg_M) {
    d8 = _Regs[reg];
  }
  else {
    d8 = _getMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]));
  }
  d8 = d8 ^ 0xFF;
  d16 = d8 + 1;
  d16 = _Regs[_Reg_A] + d16;
  d8 = lowByte(d16);
  if (d16 > 0xFF) {
    _setFlags_C(0);
  }
  else {
    _setFlags_C(1);
  }
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  if ((_Regs[_Reg_A] & 0xF) + (d8 & 0xF) > 0xF) {
    _setFlags_A(1);
  }
  else {
    _setFlags_A(0);
  }
  _Regs[_Reg_A] = d8;
  _PC++;
}

//SUI
void _I8080_SUI() {
  uint16_t d16;
  uint8_t d8;
  _PC++;
  d8 = _getMEM(_PC);
  d8 = d8 ^ 0xFF;
  d16 = d8 + 1;
  d16 = _Regs[_Reg_A] + d16;
  d8 = lowByte(d16);
  if (d16 > 0xFF) {
    _setFlags_C(0);
  }
  else {
    _setFlags_C(1);
  }
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  if ((_Regs[_Reg_A] & 0xF) + (d8 & 0xF) > 0xF) {
    _setFlags_A(1);
  }
  else {
    _setFlags_A(0);
  }
  _Regs[_Reg_A] = d8;
  _PC++;
}

//SBB
void _I8080_SBB(uint8_t reg) {
  uint16_t d16;
  uint8_t d8;
  if (reg != _Reg_M) {
    d8 = _Regs[reg];
  }
  else {
    d8 = _getMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]));
  }
  d8 = d8 + _getFlags_C();
  d8 = d8 ^ 0xFF;
  d16 = d8 + 1;
  d16 = _Regs[_Reg_A] + d16;
  d8 = lowByte(d16);
  if (d16 > 0xFF) {
    _setFlags_C(0);
  }
  else {
    _setFlags_C(1);
  }
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  if ((_Regs[_Reg_A] & 0xF) + (d8 & 0xF) > 0xF) {
    _setFlags_A(1);
  }
  else {
    _setFlags_A(0);
  }
  _Regs[_Reg_A] = d8;
  _PC++;
}

//SBI
void _I8080_SBI() {
  uint16_t d16;
  uint8_t d8;
  _PC++;
  d8 = _getMEM(_PC);
  d8 = d8 + _getFlags_C();
  //complement d8
  d8 = d8 ^ 0xFF;
  d16 = d8 + 1;
  d16 = _Regs[_Reg_A] + d16;
  d8 = lowByte(d16);
  if (d16 > 0xFF) {
    _setFlags_C(0);
  }
  else {
    _setFlags_C(1);
  }
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  if ((_Regs[_Reg_A] & 0xF) + (d8 & 0xF) > 0xF) {
    _setFlags_A(1);
  }
  else {
    _setFlags_A(0);
  }
  _Regs[_Reg_A] = d8;
  _PC++;
}

//The Auxiliary Carry bit will be affected by all addition,
//subtraction, increment, decrement, and compare
//instructions.

//INR
void _I8080_INR(uint8_t reg) {
  uint8_t d8;
  if (reg == _Reg_M)
  {
    d8 = _getMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]));
  }
  else {
    d8 = _Regs[reg];
  }
  d8++;
  if (reg == _Reg_M)
  {
     _setMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]),d8);
  }
  else {
    _Regs[reg] = d8;
  }
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  _setFlags_C(0);
  //AC
  if ((d8 & 0x0F)==0x00) {
    _setFlags_A(1);
  }
  else {
    _setFlags_A(0);
  }
  _PC++;
}

//DCR
void _I8080_DCR(uint8_t reg) {
  uint8_t d8;
  if (reg == _Reg_M)
  {
    d8 = _getMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]));
  }
  else {
    d8 = _Regs[reg];
  }
  d8--;
  if (reg == _Reg_M)
  {
     _setMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]), d8);
  }
  else {
    _Regs[reg] = d8;
  }
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  _setFlags_C(0);
  if (!((d8 & 0x0F)==0x0F)) {
    _setFlags_A(1);
  }
  else {
    _setFlags_A(0);
  }
  _PC++;
}

//INX
void _I8080_INX(uint8_t rp) {
  uint16_t d16;
  switch (rp)  {
    case _RP_BC:
      d16 = _Regs[_Reg_B] * 256 + _Regs[_Reg_C];
      d16++;
      _Regs[_Reg_B] = highByte(d16);
      _Regs[_Reg_C] = lowByte(d16);
      break;
    case _RP_DE:
      d16 = _Regs[_Reg_D] * 256 + _Regs[_Reg_E];
      d16++;
      _Regs[_Reg_D] = highByte(d16);
      _Regs[_Reg_E] = lowByte(d16);
      break;
    case _RP_HL:
      d16 = _Regs[_Reg_H] * 256 + _Regs[_Reg_L];
      d16++;
      _Regs[_Reg_H] = highByte(d16);
      _Regs[_Reg_L] = lowByte(d16);
      break;
    case _RP_SP:
      d16 = _SP;
      d16++;
      _SP = d16;
      break;
  }
  _PC++;
}

//DCX
void _I8080_DCX(uint8_t rp) {
  uint16_t d16;
  switch (rp)  {
    case _RP_BC:
      d16 = _Regs[_Reg_B] * 256 + _Regs[_Reg_C];
      d16--;
      _Regs[_Reg_B] = highByte(d16);
      _Regs[_Reg_C] = lowByte(d16);
      break;
    case _RP_DE:
      d16 = _Regs[_Reg_D] * 256 + _Regs[_Reg_E];
      d16--;
      _Regs[_Reg_D] = highByte(d16);
      _Regs[_Reg_E] = lowByte(d16);
      break;
    case _RP_HL:
      d16 = _Regs[_Reg_H] * 256 + _Regs[_Reg_L];
      d16--;
      _Regs[_Reg_H] = highByte(d16);
      _Regs[_Reg_L] = lowByte(d16);
      break;
    case _RP_SP:
      d16 = _SP;
      d16--;
      _SP = d16;
      break;
  }
  _PC++;
}

//DAD
void _I8080_DAD(uint8_t rp) {
  uint16_t d16_1;
  uint16_t d16_2;
  uint16_t d16_3;
  d16_1 = _Regs[_Reg_H] * 256 + _Regs[_Reg_L];
  switch (rp) {
    case _RP_BC:
      d16_2 = _Regs[_Reg_B] * 256 + _Regs[_Reg_C];
      break;
    case _RP_DE:
      d16_2 = _Regs[_Reg_D] * 256 + _Regs[_Reg_E];
      break;
    case _RP_HL:
      d16_2 = _Regs[_Reg_H] * 256 + _Regs[_Reg_L];
      break;
    case _RP_SP:
      d16_2 = _SP;
      break;
  }
  d16_3 = d16_1 + d16_2;
  _Regs[_Reg_H] = highByte(d16_3);
  _Regs[_Reg_L] = lowByte(d16_3);
  if ((d16_3 < d16_1) || (d16_3 < d16_2)) {
    _setFlags_C(1);
  }
  else {
    _setFlags_C(0);
  }
  _PC++;
}


//DAA
void _I8080_DAA() {
  uint8_t d8;
  d8 = 0;
  if  ( ((_Regs[_Reg_A] & 0x0F) > 9) || (_getFlags_A() == 1))
  {
    if ((_Regs[_Reg_A] & 0x0F) > 9) {
      _setFlags_A(1);
    }
    else
    {
      _setFlags_A(0);
    }
    d8 = 6;
  }
  if  ( ( ((_Regs[_Reg_A] & 0xF0) >> 4) > 9) || (_getFlags_C() == 1) || ( ( ((_Regs[_Reg_A] & 0xF0) >> 4) >= 9) && ((_Regs[_Reg_A] & 0x0F) > 9) )  )
  {
    if ( ( ((_Regs[_Reg_A] & 0xF0) >> 4) > 9) || ( ( ((_Regs[_Reg_A] & 0xF0) >> 4) >= 9) && ((_Regs[_Reg_A] & 0x0F) > 9) ) ) {
      _setFlags_C(1);
    }
    d8 = d8 | 0x60;
  }
  _Regs[_Reg_A] = _Regs[_Reg_A] + d8;
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & ZP_RESET) | (pgm_read_byte_near(SZP_table + _Regs[_Reg_A]) & B01111111);
  _PC++;
}

//ANA
void _I8080_ANA(uint8_t reg) {
  uint8_t d8;
  if (reg == _Reg_M)
  {
    d8 = _getMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]));
  }
  else
  {
    d8 = _Regs[reg];
  }
  //8080 - CY = 0, AC = bits 3 _Regs[_Reg_A] OR d8
  _setFlags_C(0);
  //setFlags_A(0);
    if ( ((_Regs[_Reg_A] & B1000) | (d8 & B1000)) != 0) {
    _setFlags_A(1);
  }
  else
  {
    _setFlags_A(0);
  }
  //8085 - CY = 0, AC = 1
  d8 = d8 & _Regs[_Reg_A];
  _Regs[_Reg_A] = d8;
  //flags
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  _PC++;
}

//ANI
void _I8080_ANI() {
  uint8_t d8;
  _PC++;
  d8 = _getMEM(_PC);
  //8080 - CY = 0, AC = bits 3 _Regs[_Reg_A] OR d8
  _setFlags_C(0);
  //_setFlags_A(0);
    if ( ((_Regs[_Reg_A] & B1000) | (d8 & B1000)) != 0) {
    _setFlags_A(1);
  }
  else
  {
    _setFlags_A(0);
  }
  d8 = d8 & _Regs[_Reg_A];
  _Regs[_Reg_A] = d8;
  //flags
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  _PC++;
}

//ORA
void _I8080_ORA(uint8_t reg) {
  uint8_t d8;
  if (reg == _Reg_M)
  {
    d8 = _getMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]));
  }
  else
  {
    d8 = _Regs[reg];
  }
  d8 = d8 | _Regs[_Reg_A];
  _Regs[_Reg_A] = d8;
  //flags
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  //CY = 0, AC = 0
  _setFlags_C(0);
  _setFlags_A(0);
  _PC++;
}

//ORI
void _I8080_ORI() {
  uint8_t d8;
  _PC++;
  d8 = _getMEM(_PC);
  d8 = d8 | _Regs[_Reg_A];
  _Regs[_Reg_A] = d8;
  //flags
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  //CY = 0, AC = 0
  _setFlags_C(0);
  _setFlags_A(0);
  _PC++;
}

//XRA
void _I8080_XRA(uint8_t reg) {
  uint8_t d8;
  if (reg == _Reg_M)
  {
    d8 = _getMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]));
  }
  else
  {
    d8 = _Regs[reg];
  }
  d8 = d8 ^ _Regs[_Reg_A];
  _Regs[_Reg_A] = d8;
  //flags
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  //CY = 0, AC = 0
  _setFlags_C(0);
  _setFlags_A(0);
  _PC++;
}

//XRI
void _I8080_XRI() {
  uint8_t d8;
  _PC++;
  d8 = _getMEM(_PC);
  d8 = d8 ^ _Regs[_Reg_A];
  _Regs[_Reg_A] = d8;
  //flags
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + d8);
  //CY = 0, AC = 0
  _setFlags_C(0);
  _setFlags_A(0);
  _PC++;
}

//CMP
void _I8080_CMP(uint8_t reg) {
  uint16_t d16;
  uint8_t d8;
  if (reg != _Reg_M) {
    d8 = _Regs[reg];
  }
  else {
    d8 = _getMEM(word(_Regs[_Reg_H], _Regs[_Reg_L]));
  }
  if (_Regs[_Reg_A] < d8) {
    _setFlags_C(1);
  }
  else {
    _setFlags_C(0);
  }
  d8 = d8 ^ 0xFF;
  d16 = d8 + 1;
  d16 = _Regs[_Reg_A] + d16;
    //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + lowByte(d16));
  if ((_Regs[_Reg_A] & 0xF) + (d8 & 0xF) > 0xF) {
    _setFlags_A(1);
  }
  else {
    _setFlags_A(0);
  }
  _PC++;
}

//CPI
void _I8080_CPI() {
  uint16_t d16;
  uint8_t d8;
  _PC++;
  d8 = _getMEM(_PC);
  d8 = d8 ^ 0xFF;
  d16 = d8 + 1;
  d16 = _Regs[_Reg_A] + d16;
  if (d16 > 0xFF) {
    _setFlags_C(0);
  }
  else {
    _setFlags_C(1);
  }
  //SZP flags
  _Regs[_Reg_M] = (_Regs[_Reg_M] & SZP_RESET) | pgm_read_byte_near(SZP_table + lowByte(d16));
  if ((_Regs[_Reg_A] & 0xF) + (d8 & 0xF) > 0xF) {
    _setFlags_A(1);
  }
  else {
    _setFlags_A(0);
  }
  _PC++;
}

//RRC
void _I8080_RRC() {
  uint8_t d1;
  d1 = bitRead(_Regs[_Reg_A], 0);
  _setFlags_C(bitRead(_Regs[_Reg_A], 0));
  _Regs[_Reg_A] = _Regs[_Reg_A] >> 1;
  bitWrite(_Regs[_Reg_A], 7, d1);
  _PC++;
}

//RLC
void _I8080_RLC() {
  uint8_t d1;
  d1 = bitRead(_Regs[_Reg_A], 7);
  _setFlags_C(bitRead(_Regs[_Reg_A], 7));
  _Regs[_Reg_A] = _Regs[_Reg_A] << 1;
  bitWrite(_Regs[_Reg_A], 0, d1);
  _PC++;
}

//RAL
void _I8080_RAL() {
  uint8_t d1;
  d1 = _getFlags_C();
  _setFlags_C(bitRead(_Regs[_Reg_A], 7));
  _Regs[_Reg_A] = _Regs[_Reg_A] << 1;
  bitWrite(_Regs[_Reg_A], 0, d1);
  _PC++;
}

//RAR
void _I8080_RAR() {
  uint8_t d1;
  d1 = _getFlags_C();
  _setFlags_C(bitRead(_Regs[_Reg_A], 0));
  _Regs[_Reg_A] = _Regs[_Reg_A] >> 1;
  bitWrite(_Regs[_Reg_A], 7, d1);
  _PC++;
}

//CMA
void _I8080_CMA() {
  _Regs[_Reg_A] = ~_Regs[_Reg_A];
  _PC++;
}

//CMC
void _I8080_CMC() {
  if ((_Regs[_Reg_M] & 0x1) != 0)
  {
    _setFlags_C(0);
  }
  else {
    _setFlags_C(1);
  }
  _PC++;
}

//STC
void _I8080_STC() {
  _setFlags_C(1);
  _PC++;
}

//JMP
void _I8080_JMP() {
  _PC = pc2a16();
}

//JNZ
void _I8080_JNZ()
{
  uint16_t a16;
  a16 = pc2a16();
  if (_getFlags_Z() == 0) {
    _PC = a16;
  }
  else {
    _PC++;
  }
}

//JZ
void _I8080_JZ()
{
  uint16_t a16;
  a16 = pc2a16();
  if (_getFlags_Z() == 1) {
    _PC = a16;
  }
  else {
    _PC++;
  }
}

//JNC
void _I8080_JNC()
{
  uint16_t a16;
  a16 = pc2a16();
  if (_getFlags_C() == 0) {
    _PC = a16;
  }
  else {
    _PC++;
  }
}

//JC
void _I8080_JC()
{
  uint16_t a16;
  a16 = pc2a16();
  if (_getFlags_C() == 1) {
    _PC = a16;
  }
  else {
    _PC++;
  }
}

//JPO
void _I8080_JPO()
{
  uint16_t a16;
  a16 = pc2a16();
  if (_getFlags_P() == 0) {
    _PC = a16;
  }
  else {
    _PC++;
  }
}

//JPE
void _I8080_JPE()
{
  uint16_t a16;
  a16 = pc2a16();
  if (_getFlags_P() == 1) {
    _PC = a16;
  }
  else {
    _PC++;
  }
}

//JP
void _I8080_JP()
{
  uint16_t a16;
  a16 = pc2a16();
  if (_getFlags_S() == 0) {
    _PC = a16;
  }
  else {
    _PC++;
  }
}

//JM
void _I8080_JM()
{
  uint16_t a16;
  a16 = pc2a16();
  if (_getFlags_S() == 1) {
    _PC = a16;
  }
  else {
    _PC++;
  }
}

//CALL
void _I8080_CALL() {
  uint16_t a16;
  a16 = pc2a16();
  _PC++;
  pc2sp();
  _PC = a16;
}

//CNZ
void _I8080_CNZ() {
  uint16_t a16;
  a16 = pc2a16();
  _PC++;
  if (_getFlags_Z() == 0) {
    pc2sp();
    _PC = a16;
  }
}

//CZ
void _I8080_CZ() {
  uint16_t a16;
  a16 = pc2a16();
  _PC++;
  if (_getFlags_Z() == 1) {
    pc2sp();
    _PC = a16;
  }
}

//CNC
void _I8080_CNC() {
  uint16_t a16;
  a16 = pc2a16();
  _PC++;
  if (_getFlags_C() == 0) {
    pc2sp();
    _PC = a16;
  }
}

//CC
void _I8080_CC() {
  uint16_t a16;
  a16 = pc2a16();
  _PC++;
  if (_getFlags_C() == 1) {
    pc2sp();
    _PC = a16;
  }
}

//CPO
void _I8080_CPO() {
  uint16_t a16;
  a16 = pc2a16();
  _PC++;
  if (_getFlags_P() == 0) {
    pc2sp();
    _PC = a16;
  }
}

//CPE
void _I8080_CPE() {
  uint16_t a16;
  a16 = pc2a16();
  _PC++;
  if (_getFlags_P() == 1) {
    pc2sp();
    _PC = a16;
  }
}

//CP
void _I8080_CP() {
  uint16_t a16;
  a16 = pc2a16();
  _PC++;
  if (_getFlags_S() == 0) {
    pc2sp();
    _PC = a16;
  }
}

//CM
void _I8080_CM() {
  uint16_t a16;
  a16 = pc2a16();
  _PC++;
  if (_getFlags_S() == 1) {
    pc2sp();
    _PC = a16;
  }
}

//RET
void _I8080_RET()
{
  _PC = sp2a16();
}

//RNZ
void _I8080_RNZ()
{
  if (_getFlags_Z() == 0) {
    _PC = sp2a16();
  }
  else {
    _PC++;
  }
}

//RZ
void _I8080_RZ() {
  if (_getFlags_Z() == 1) {
    _PC = sp2a16();
  }
  else {
    _PC++;
  }
}

//RNC
void _I8080_RNC()
{
  if (_getFlags_C() == 0) {
    _PC = sp2a16();
  }
  else {
    _PC++;
  }
}

//RC
void _I8080_RC() {
  if (_getFlags_C() == 1) {
    _PC = sp2a16();
  }
  else {
    _PC++;
  }
}

//RPO
void _I8080_RPO() {
  if (_getFlags_P() == 0) {
    _PC = sp2a16();
  }
  else {
    _PC++;
  }
}

//RPE
void _I8080_RPE() {
  if (_getFlags_P() == 1) {
    _PC = sp2a16();
  }
  else {
    _PC++;
  }
}

//RP
void _I8080_RP() {
  if (_getFlags_S() == 0) {
    _PC = sp2a16();
  }
  else {
    _PC++;
  }
}

//RM
void _I8080_RM() {
  if (_getFlags_S() == 1) {
    _PC = sp2a16();
  }
  else {
    _PC++;
  }
}

//RST
void _I8080_RST(uint8_t n) {
  _PC++;
  pc2sp();
  _PC = n * 8;
}


//PCHL
void _I8080_PCHL() {
  uint16_t a16;
  a16 = _Regs[_Reg_L];
  a16 = a16 + 256 * _Regs[_Reg_H];
  _PC = a16;
}

//XTHL
void _I8080_XTHL() {
  uint16_t d16;
  d16 = _getMEM(_SP);
  d16 = d16 + _getMEM(_SP + 1) * 256;
  _setMEM(_SP, _Regs[_Reg_L]);
  _setMEM(_SP + 1, _Regs[_Reg_H]);
  _Regs[_Reg_L] = lowByte(d16);
  _Regs[_Reg_H] = highByte(d16);
  _PC++;
}

//SPHL
void _I8080_SPHL()
{
  _SP = _Regs[_Reg_L] + 256 * _Regs[_Reg_H];
  _PC++;
}

//PUSH
void _I8080_PUSH(uint8_t rp) {
  uint16_t d16;
  _SP--;
  _SP--;
  switch (rp)  {
    case _RP_BC:
      d16 = _Regs[_Reg_B] * 256 + _Regs[_Reg_C];
      break;
    case _RP_DE:
      d16 = _Regs[_Reg_D] * 256 + _Regs[_Reg_E];
      break;
    case _RP_AF:
      d16 = _Regs[_Reg_A] * 256 + _Regs[_Reg_M];
      break;
    case _RP_HL:
      d16 = _Regs[_Reg_H] * 256 + _Regs[_Reg_L];
      break;
  }
  _setMEM(_SP, lowByte(d16));
  _setMEM(_SP + 1, highByte(d16));
  _PC++;
}

//POP
void _I8080_POP(uint8_t rp) {
  uint16_t d16;
  d16 = _getMEM(_SP);
  _SP++;
  d16 = d16 + _getMEM(_SP) * 256;
  switch (rp)  {
    case _RP_BC:
      _Regs[_Reg_B] = highByte(d16);
      _Regs[_Reg_C] = lowByte(d16);
      break;
    case _RP_DE:
      _Regs[_Reg_D] = highByte(d16);
      _Regs[_Reg_E] = lowByte(d16);
      break;
    case _RP_AF:
      _Regs[_Reg_A] = highByte(d16);
      _Regs[_Reg_M] = lowByte(d16);
      break;
    case _RP_HL:
      _Regs[_Reg_H] = highByte(d16);
      _Regs[_Reg_L] = lowByte(d16);
      break;
  }
  _SP++;
  _PC++;
}

//IN
void _I8080_IN() {
  uint8_t d8;
  uint8_t pa;
  _PC++;
  pa = _getMEM(_PC);
  d8 = in_port(pa);
  _Regs[_Reg_A] = d8;
  _PC++;
}

//OUT
void _I8080_OUT() {
  uint8_t d8;
  uint8_t pa;
  _PC++;
  pa = _getMEM(_PC);
  d8 = _Regs[_Reg_A];
  out_port(pa, d8);
  _PC++;
}

//EI
void _I8080_EI() {
  INTE = true;
  _PC++;
}

//DI
void _I8080_DI() {
  INTE = false;
  _PC++;
}


