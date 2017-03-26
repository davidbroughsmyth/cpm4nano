/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Version 0.1
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*   
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*   
*   CREDITS
*   CP/M 2.2 sources - http://www.cpm.z80.de/source.html
*   CP/M 2.2 binaries - http://www.retroarchive.org/cpm/
*   CP/M 2.2 & DRI binaries - http://www.gaby.de/cpm/binary.html
*   binary file to a c/c++ array converter - http://tools.garry.tv/bin2c/
*   Pretty 8080 Assembler - http://asdasd.rpg.fi/~svo/i8080/
*   Cristian Maglie <c.maglie@bug.st> - SPI Master library for arduino
*   William Greiman - Arduino Sd2Card Library
*/

//TO DO

#include <avr/pgmspace.h>
#include "Sd2Card.h"
//include "TEST.h"
#include "CPM_def.h"


const static uint8_t PROGMEM parity_table[] = {
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
};

const static uint8_t PROGMEM memtest_table[] = {
    0x3D, 0x55, 0x5F, 0x15, 0x23, 0x47, 0x1C, 0x31, 0x48, 0x60, 0x35, 0x11, 0x4F, 0x2F, 0x2E, 0x14, 0x20, 0x5B, 0x39, 0x26, 0x09, 0x61, 0x34, 0x30, 0x50, 0x2B, 0x4B, 0x0F, 0x63, 0x1F, 0x10, 0x1E, 0x36,
};
const uint16_t MEMTEST_TABLE_SIZE = 33;


Sd2Card card;

boolean DEBUG;//debug mode flag
uint32_t RAM_SIZE = 65536;

const uint16_t CACHE_LINE_SIZE = 64;//128 byte max
const uint8_t CACHE_LINES_NUM = 4;
const uint16_t CACHE_SIZE = CACHE_LINES_NUM * CACHE_LINE_SIZE;
uint32_t cache_tag[CACHE_LINES_NUM];
uint16_t cache_start[CACHE_LINES_NUM];
boolean cache_dirty[CACHE_LINES_NUM];                             
static uint8_t cache[CACHE_SIZE];

const uint16_t SD_BLK_SIZE = 128;

static unsigned char _buffer[SD_BLK_SIZE];
static unsigned char _dsk_buffer[SD_BLK_SIZE];

uint16_t breakpoint = 0xFFFF;
bool exitFlag = false;

char inChar;
const int KbdBufferSize = 32;
char KbdBuffer[KbdBufferSize + 1];
int KbdPtr = 0;



boolean Z80 = false;//Z80 emulation

//console emulation
//console ports
const uint8_t CON_PORT_STATUS = 0x00;//status
const uint8_t CON_PORT_DATA = 0x01;//data
//-----------------------------------------------------
//FDD emulation
//FDD controller ports
const uint8_t FDD_BASE = 0xE0;//FDD base address
const uint8_t FDD_PORT_CMD = FDD_BASE+0;//status/command
const uint8_t FDD_PORT_TRK = FDD_BASE+1;//track
const uint8_t FDD_PORT_SEC = FDD_BASE+2;//sector
const uint8_t FDD_PORT_DRV = FDD_BASE+3;//drive select
//DMA controller ports
const uint8_t FDD_PORT_DMA_ADDR_LO = FDD_BASE+4;//DMA address low byte
const uint8_t FDD_PORT_DMA_ADDR_HI = FDD_BASE+5;//DMA address high byte
//FDD commands
const uint8_t FDD_RD_CMD = 0x00; //read sector command
const uint8_t FDD_WRT_CMD = 0x01; //write sector command
//FDD registers
uint8_t FDD_REG_SEC=1;//sector register
uint8_t FDD_REG_TRK=0;//track register
uint8_t FDD_REG_DRV=0;//drive register
boolean FDD_REG_STATUS=false;//true - O.K., false - ERROR
uint16_t FDD_REG_DMA=0;//DMA address register
//-----------------------------------------------------
//aux ports
boolean LED_on = false;
uint8_t LED_count;
const uint8_t LED_delay = 3;
const uint8_t LED_pin = 9;//D9 pin - LED
const uint8_t IN_pin = 8;//D8 pin - IN
const uint8_t OUT_pin = 7;//D7 pin - OUT
const uint8_t IN_PORT = 0xF0;//IN port
const uint8_t OUT_PORT = 0xF1;//OUT port
//-----------------------------------------------------
//SD read/write

uint8_t readSD (uint32_t blk, uint16_t offset) {
  uint8_t res;
  res = card.readBlock(blk, _buffer, offset);
  digitalWrite(LED_pin,HIGH);
  //LEDon = true;
  //LEDcount = LEDdelay;
  return res;
}

uint8_t writeSD (uint32_t blk) {
  uint8_t res;
  res = card.writeBlock(blk, _buffer);
  digitalWrite(LED_pin,HIGH);
  LED_on = true;
  LED_count = LED_delay;
  return res;
}

//-----------------------------------------------------
//keyboard monitor procedures

//keys codes
const uint8_t BS_KEY=0x08;
const uint8_t DEL_KEY=0x7F;
const uint8_t CTRL_Q_KEY=0x11;
const uint8_t CTRL_C_KEY=0x03;
const uint8_t CTRL_Z_KEY=0x1A;
const uint8_t CTRL_O_KEY=0x0F;
const uint8_t CTRL_X_KEY=0x18;

int str2hex(String s)
{
  int x = 0;
  for (int i = 0; i < s.length(); i++) {
    char c = s.charAt(i);
    if (c >= '0' && c <= '9') {
      x *= 16;
      x += c - '0';
    }
    else if (c >= 'A' && c <= 'F') {
      x *= 16;
      x += (c - 'A') + 10;
    }
    else break;
  }
  return x;
}

uint8_t chr2hex(char c)
{
  uint8_t x = 0;
  if (c >= '0' && c <= '9') {
    x *= 16;
    x += c - '0';
  }
  else if (c >= 'A' && c <= 'F') {
    x *= 16;
    x += (c - 'A') + 10;
  }
  return x;
}

boolean hexcheck(uint8_t start, uint8_t len) {
  uint8_t i;
  boolean ok;
  ok = true;
  for(i=start;i<(start+len);i++) {
    if (!(((KbdBuffer[i]>='0') && (KbdBuffer[i]<='9')) || ((KbdBuffer[i]>='A') && (KbdBuffer[i]<='F')))) {
      ok = false;
    }
  }
  return ok;
}

uint8_t kbd2byte(uint8_t start) {
  return chr2hex(KbdBuffer[start])*16+chr2hex(KbdBuffer[start+1]);
}

uint16_t kbd2word(uint8_t start) {
  return (uint16_t)(chr2hex(KbdBuffer[start])*16*16*16) + (uint16_t)(chr2hex(KbdBuffer[start+1])*16*16) + (uint16_t)(chr2hex(KbdBuffer[start+2])*16) + (uint16_t)(chr2hex(KbdBuffer[start+3]));
}

char upCase(char symbol) {
  if (symbol >= 97 && symbol <= 122) {
        symbol = char(uint8_t(symbol) - 32);
  }
  return symbol;
}

//-----------------------------------------------------

#include "MEM.h"
#include "i8080_exec.h"
#include "ESC.h"
#include "BIOS.h"

void call(word addr)
{
  byte cmd;
  bool exe_flag;
  uint8_t i;
  exitFlag = false;
  _PC = addr;
  do
  {
if (_PC ==  breakpoint) {
  DEBUG = true;
}

#include "BIOS_int.h"
cmd = _getMEM(_PC);
#include "debug.h"
#include "i8080_cmds.h"
  } while (exitFlag == false); 
}

void setup() {
  uint32_t i;
  uint16_t j;
  uint16_t k;
  uint32_t _cardsize;
  uint8_t res;
  bool RAMTestPass = true;
  // start serial port at 9600 bps
  Serial.begin(9600);
  while (!Serial) {
    ; //wait for serial port to connect. Needed for native USB port only
  }
  delay(1000);
  clrscr();
  //pins setup
  pinMode(LED_pin, OUTPUT);
  digitalWrite(LED_pin,LOW);
  pinMode(IN_pin, INPUT);
  pinMode(OUT_pin, OUTPUT);
  digitalWrite(OUT_pin,LOW);
  //Timer1 setup
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 312;//50 Hz
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);//prescaler 1024
  TIMSK1 |= (1 << OCIE1A);
  sei();
  //logo
  color(2);
  Serial.println(F("***************************"));
  Serial.println(F("*  CP/M for Arduino Nano  *"));
  Serial.println(F("*    (C) 2017 FoxyLab     *"));
  Serial.println(F(" https://acdc.foxylab.com *"));
  Serial.println(F("***************************"));
  Serial.println("");
  //cache init
  for (i = 0; i < CACHE_LINES_NUM; i++) {
    cache_tag[i] = 0xFFFFFFFF;
  }
  for (i = 0; i < CACHE_LINES_NUM; i++) {
    cache_dirty[i] = true;
  }
  for (i = 0; i < CACHE_LINES_NUM; i++) {
    cache_start[i] = i * CACHE_LINE_SIZE;
  }
  //SD card init
  do {
    card.init(SPI_HALF_SPEED, 10);
    _cardsize = card.cardSize();
    if (_cardsize !=0) {
      Serial.println(F("Card size: "));
      Serial.print(_cardsize);
      Serial.println(F(" sectors"));
    }
    else {
      delay(250);
    }  
  } while (_cardsize == 0);
  //RAM test
  Serial.print(F("RAM test..."));
  //RAM write
  j = 0;
  for (i = 0; i < RAM_SIZE ; i++) {
    _setMEM(i, pgm_read_byte_near(memtest_table+j));
    if ((i % 16384) == 0) {
      Serial.println("");
    }
    if ((i % 1024) == 0) {
      Serial.print("*");
    }
    j++;
    if (j == MEMTEST_TABLE_SIZE) {
      j = 0;
    }
  }
  //RAM read
  j = 0;
  for (i = 0; i < RAM_SIZE; i++) {
    if ((i % 16384) == 0) {
      Serial.println("");
    }
    if ((i % 1024) == 0) {
      Serial.print("?");
    }
    if (_getMEM(i) != pgm_read_byte_near(memtest_table+j)) {
      RAMTestPass = false;
      Serial.print(F("RAM test failed at byte "));
      Serial.println(i, HEX);
      RAM_SIZE = i;
      break;
    }
    j++;
    if (j == MEMTEST_TABLE_SIZE) {
      j = 0;
    }
  }
  Serial.println("");
  if (RAMTestPass == true) {
    Serial.println(F("O.K."));
  }
  Serial.print(i, DEC);
  Serial.println(F(" byte(s) of RAM are available"));
  //RAM clear
  Serial.print(F("RAM clearing..."));
  for (i = 0; i < RAM_SIZE; i++) {
    _setMEM(i, 0);
    if ((i % 16384) == 0) {
      Serial.println("");
    }
    if ((i % 1024) == 0) {
      Serial.print("#");
    }
  }
  Serial.println("");
  color(9);
  //stack init
  _SP = SP_INIT;
  DEBUG = true;//debug on
  delay(2000);
  clrscr();//clear screen
  xy(MON_Y, 0);//cursor positioning
  Serial.print('>');
}

//Timer1 interrupt
ISR(TIMER1_COMPA_vect){ 
  if (LED_on) {
    LED_count--;
    if (LED_count==0) {
      LED_on = false;
      digitalWrite(LED_pin,LOW);//LED off  
    }
  }
}

void loop() {
  uint32_t adr;
  uint8_t port;
  uint8_t dat;
  uint8_t i;
  boolean _EOF;
  boolean error;
  uint8_t hex_count;
  uint8_t hex_len;
  uint8_t hex_type;
  uint8_t hex_crc;
  uint8_t hex_bytes;
  uint16_t len;
  uint16_t count;
  uint8_t crc;
  uint8_t tmp_byte;
  uint16_t tmp_word;
  while (true) {
    do
    {
      inChar = '\0';
      if (Serial.available() > 0) {
        inChar = Serial.read();
        //Serial.print(uint8_t(inChar),HEX);
        inChar = upCase(inChar);
        if (uint8_t(inChar)==BS_KEY) {
          //backspace
          if (KbdPtr>0) {
            KbdPtr--;
            KbdBuffer[KbdPtr] = '\0'; 
          }
        }
        else {
          KbdBuffer[KbdPtr] = inChar;
          KbdPtr++;
        }
        Serial.write(inChar);
      }
    } while ((inChar != '\r') && (inChar != '\n') && (KbdPtr < KbdBufferSize));
    Serial.print('\n');
    KbdPtr = 0;
    #include "mon.h"
    xy(MON_Y, 0);//cursor positioning
    Serial.print('>');
    clrend();//clear input line to end
  }
}




