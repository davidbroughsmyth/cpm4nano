/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0
    Version 0.1
    Tested with Arduino IDE 1.6.6
    Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab
    Email:    support@foxylab.com
    Website:  https://acdc.foxylab.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

    CREDITS
    CP/M 2.2 sources - http://www.cpm.z80.de/source.html
    CP/M 2.2 binaries - http://www.retroarchive.org/cpm/
    CP/M 2.2 & DRI binaries - http://www.gaby.de/cpm/binary.html
    binary file to a c/c++ array converter - http://tools.garry.tv/bin2c/
    Pretty 8080 Assembler - http://asdasd.rpg.fi/~svo/i8080/
    Cristian Maglie <c.maglie@bug.st> - SPI Master library for arduino
    William Greiman - Arduino Sd2Card Library
    Christian Weichel <info@32leaves.net> - PS2Keyboard library
*/

//TO DO
//memory constants/variables
const uint8_t MEM_SIZE = 64;//System RAM Size, KBytes
const uint16_t MEM_MAX = (MEM_SIZE-1)*1024U + 1023U;//maximum system RAM address
uint32_t RAM_AVAIL = 0x10000L;//available RAM Size, KBytes (64 KBytes maximum)
const uint8_t RAM_SIZE = 64;//RAM Size for CP/M, KBytes

#include <avr/pgmspace.h>
#include "Sd2Card.h"
//include "TEST.h"
#include "CPM_def.h"
#include "PS2Keyboard.h"
#include "EEPROM.h"
#include "ESC.h"

//version
const char VER_MAJOR = '0';
const char VER_MINOR = '3';
//----------------------------------------------------
//CPU
//registers
volatile uint8_t _W;// W register
volatile uint8_t _Z;// Z register
volatile uint8_t _ACT;// ACT register
volatile uint8_t _TMP;// TMP register
volatile uint8_t _ALU;// ALU
volatile uint16_t _PC; //program counter
volatile uint16_t _SP; //stack pointer
volatile uint8_t _IR; //instruction register
volatile uint8_t _DB; //data bus buffer
volatile uint16_t _AB; //data bus buffer
bool INTE;
boolean DEBUG;//debug mode flag
volatile bool exitFlag = false;
//SZ000P00  SZP flags lookup table
const static uint8_t PROGMEM SZP_table[] = {
  B01000100, B00000000, B00000000, B00000100, B00000000, B00000100, B00000100, B00000000, B00000000, B00000100, B00000100, B00000000, B00000100, B00000000, B00000000, B00000100,
  B00000000, B00000100, B00000100, B00000000, B00000100, B00000000, B00000000, B00000100, B00000100, B00000000, B00000000, B00000100, B00000000, B00000100, B00000100, B00000000,
  B00000000, B00000100, B00000100, B00000000, B00000100, B00000000, B00000000, B00000100, B00000100, B00000000, B00000000, B00000100, B00000000, B00000100, B00000100, B00000000,
  B00000100, B00000000, B00000000, B00000100, B00000000, B00000100, B00000100, B00000000, B00000000, B00000100, B00000100, B00000000, B00000100, B00000000, B00000000, B00000100,
  B00000000, B00000100, B00000100, B00000000, B00000100, B00000000, B00000000, B00000100, B00000100, B00000000, B00000000, B00000100, B00000000, B00000100, B00000100, B00000000,
  B00000100, B00000000, B00000000, B00000100, B00000000, B00000100, B00000100, B00000000, B00000000, B00000100, B00000100, B00000000, B00000100, B00000000, B00000000, B00000100,
  B00000100, B00000000, B00000000, B00000100, B00000000, B00000100, B00000100, B00000000, B00000000, B00000100, B00000100, B00000000, B00000100, B00000000, B00000000, B00000100,
  B00000000, B00000100, B00000100, B00000000, B00000100, B00000000, B00000000, B00000100, B00000100, B00000000, B00000000, B00000100, B00000000, B00000100, B00000100, B00000000,
  B10000000, B10000100, B10000100, B10000000, B10000100, B10000000, B10000000, B10000100, B10000100, B10000000, B10000000, B10000100, B10000000, B10000100, B10000100, B10000000,
  B10000100, B10000000, B10000000, B10000100, B10000000, B10000100, B10000100, B10000000, B10000000, B10000100, B10000100, B10000000, B10000100, B10000000, B10000000, B10000100,
  B10000100, B10000000, B10000000, B10000100, B10000000, B10000100, B10000100, B10000000, B10000000, B10000100, B10000100, B10000000, B10000100, B10000000, B10000000, B10000100,
  B10000000, B10000100, B10000100, B10000000, B10000100, B10000000, B10000000, B10000100, B10000100, B10000000, B10000000, B10000100, B10000000, B10000100, B10000100, B10000000,
  B10000100, B10000000, B10000000, B10000100, B10000000, B10000100, B10000100, B10000000, B10000000, B10000100, B10000100, B10000000, B10000100, B10000000, B10000000, B10000100,
  B10000000, B10000100, B10000100, B10000000, B10000100, B10000000, B10000000, B10000100, B10000100, B10000000, B10000000, B10000100, B10000000, B10000100, B10000100, B10000000,
  B10000000, B10000100, B10000100, B10000000, B10000100, B10000000, B10000000, B10000100, B10000100, B10000000, B10000000, B10000100, B10000000, B10000100, B10000100, B10000000,
  B10000100, B10000000, B10000000, B10000100, B10000000, B10000100, B10000100, B10000000, B10000000, B10000100, B10000100, B10000000, B10000100, B10000000, B10000000, B10000100,
};
//----------------------------------------------------
//CONSOLE
uint8_t CON_IN = 0;
//0 - terminal program
//1 - PS/2 keyboard
//console ports
//SIO-A//SSM
const uint8_t SIOA_CON_PORT_STATUS = 0x00;//status
const uint8_t SIOA_CON_PORT_DATA = 0x01;//data
//SIO-2
const uint8_t SIO2_CON_PORT_STATUS = 0x10;//status
const uint8_t SIO2_CON_PORT_DATA = 0x11;//data
//-----------------------------------------------------
//FDD
//FDD controller ports
const uint8_t FDD_BASE = 0xE0;//FDD base address
const uint8_t FDD_PORT_CMD = FDD_BASE + 0; //status/command
const uint8_t FDD_PORT_TRK = FDD_BASE + 1; //track
const uint8_t FDD_PORT_SEC = FDD_BASE + 2; //sector
const uint8_t FDD_PORT_DRV = FDD_BASE + 3; //drive select
//DMA controller ports
const uint8_t FDD_PORT_DMA_ADDR_LO = FDD_BASE + 4; //DMA address low byte
const uint8_t FDD_PORT_DMA_ADDR_HI = FDD_BASE + 5; //DMA address high byte
//FDD commands
const uint8_t FDD_RD_CMD = 0x00; //read sector command
const uint8_t FDD_WRT_CMD = 0x01; //write sector command
//FDD registers
uint8_t FDD_REG_SEC = 1; //sector register
uint8_t FDD_REG_TRK = 0; //track register
uint8_t FDD_REG_DRV = 0; //drive register
boolean FDD_REG_STATUS = false; //true - O.K., false - ERROR
uint16_t FDD_REG_DMA = 0; //DMA address register
//-----------------------------------------------------
//AUX
boolean LED_on = false;
uint8_t LED_count;
const uint8_t LED_delay = 3;
const uint8_t LED_pin = 9;//D9 pin - LED
const uint8_t IN_pin = 8;//D8 pin - IN
const uint8_t OUT_pin = 7;//D7 pin - OUT
const uint8_t IN_PORT = 0xF0;//IN port
const uint8_t OUT_PORT = 0xF1;//OUT port
//-----------------------------------------------------
//SD
Sd2Card card;
const uint8_t SS_SD_pin = 10;//SS pin (D10)
const uint16_t SD_BLK_SIZE = 128;//SD block size
static unsigned char _buffer[SD_BLK_SIZE];
static unsigned char _dsk_buffer[SD_BLK_SIZE];
//block read from SD
uint8_t readSD (uint32_t blk, uint16_t offset) {
  uint8_t res;
  res = card.readBlock(blk, _buffer, offset);
  return res;
}
//block write to SD
uint8_t writeSD (uint32_t blk) {
  uint8_t res;
  res = card.writeBlock(blk, _buffer);
  if (!LED_on) {
    fastDigitalWrite(LED_pin, HIGH);
    LED_on = true;//WRITE LED on
  }
  LED_count = LED_delay;
  return res;
}
//----------------------------------------------------
//ALTAIR
uint8_t SENSE_SW = 0x00;//Altair/IMSAI sense switch default off
const uint8_t SENSE_SW_PORT = 0xFF;//Altair/IMSAI sense switch port
#include "MEM.h"
#include "i8080_exec.h"
//---------------------------------------------------
//MEMORY
const static uint8_t PROGMEM memtest_table[] = {
  0x3D, 0x55, 0x5F, 0x15, 0x23, 0x47, 0x1C, 0x31, 0x48, 0x60, 0x35, 0x11, 0x4F, 0x2F, 0x2E, 0x14, 0x20, 0x5B, 0x39, 0x26, 0x09, 0x61, 0x34, 0x30, 0x50, 0x2B, 0x4B, 0x0F, 0x63, 0x1F, 0x10, 0x1E, 0x36,
};
const uint16_t MEMTEST_TABLE_SIZE = 33;
uint8_t RAM_TEST_MODE;//memory check mpde


//FRAM read/write
/*
const uint8_t SS_SPIRAM_pin = 6; //SS SPI RAM pin D6
const uint16_t SPIRAM_DELAY_US = 100;
SpiRAM SpiRam(0, SS_SPIRAM_pin);

uint8_t readSPIRAM (uint16_t adr) {
  uint8_t dat;
  dat = uint8_t(SpiRam.read_byte(adr));
  delayMicroseconds(SPIRAM_DELAY_US);
  return dat;
}

void writeSPIRAM (uint16_t adr, uint8_t dat) {
  SpiRam.write_byte(adr, char(dat));
  delayMicroseconds(SPIRAM_DELAY_US);
}
*/
//-----------------------------------------------------
//keyboard monitor procedures
char inChar;
const int MON_BUFFER_SIZE = 32;//monitor input buffer size
char mon_buffer[MON_BUFFER_SIZE + 1];
int mon_ptr = 0;
const uint32_t SET_PAUSE = 5000;//pause (msecs)
boolean MON = true;
//keys codes
const uint8_t BS_KEY = 0x08;
const uint8_t DEL_KEY = 0x7F;
const uint8_t CTRL_Q_KEY = 0x11;
const uint8_t CTRL_C_KEY = 0x03;
const uint8_t CTRL_Z_KEY = 0x1A;
const uint8_t CTRL_O_KEY = 0x0F;
const uint8_t CTRL_X_KEY = 0x18;
const uint8_t CTRL_SLASH_KEY = 0x1F;
//console input variables
const uint8_t KBD_BUFFER_SIZE = 16;//console input buffer size
volatile char kbd_buffer[KBD_BUFFER_SIZE];//console input buffer
volatile uint8_t kbd_chars = 0;//received chars number

//console input/output procedures
char con_read() {
  char key;
  key = '\0';
  switch (CON_IN) {
      case 0: //terminal
        if (Serial.available()>0) {
          key = Serial.read();
          if (!MON && ((uint8_t)key == CTRL_SLASH_KEY)) {
            exitFlag = true;
          }
        } 
        break;
      case 1: //PS/2 keyboard
        //
        break;
  }
  return key;
}

boolean con_ready() {
  boolean res;
  switch (CON_IN) {
    case 0: //terminal
            if (Serial.available()>0) {
              res = true;
            }
            else {
              res = false;
            }
            break;
    case 1: //PS/2 keyboard
            //cli();
            if (kbd_chars>0) {
              res = true;
            }
            else {
              res = false;
            }
            //sei();
            break;
  }
  return res;
}

void con_flush() {
    switch (CON_IN) {
      case 0: //terminal
              Serial.flush();
              break;
      case 1: //PS/2 keyboard
              //cli();
              kbd_chars = 0;
              //sei();
              break;
    }
}

//conversion functions
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
  for (i = start; i < (start + len); i++) {
    if (!(((mon_buffer[i] >= '0') && (mon_buffer[i] <= '9')) || ((mon_buffer[i] >= 'A') && (mon_buffer[i] <= 'F')))) {
      ok = false;
    }
  }
  return ok;
}

//keyboard input to numbers
uint8_t kbd2nibble(uint8_t start) {
  return chr2hex(mon_buffer[start]);
}

uint8_t kbd2byte(uint8_t start) {
  return chr2hex(mon_buffer[start]) * 16 + chr2hex(mon_buffer[start + 1]);
}

uint16_t kbd2word(uint8_t start) {
  return (uint16_t)(chr2hex(mon_buffer[start]) * 16 * 16 * 16) + (uint16_t)(chr2hex(mon_buffer[start + 1]) * 16 * 16) + (uint16_t)(chr2hex(mon_buffer[start + 2]) * 16) + (uint16_t)(chr2hex(mon_buffer[start + 3]));
}

char upCase(char symbol) {
  if (symbol >= 97 && symbol <= 122) {
    symbol = char(uint8_t(symbol) - 32);
  }
  return symbol;
}

//-----------------------------------------------------
//EEPRPOM
/*
EEPROM cells
               0xFE - 0x55
               0xFF - 0xAA
               0x00 - drive A
               0x01 - drive B
               0x02 - drive C
               0x03 - drive D
               0x04 - sense sw
*/
//EEPROM init
const int EEPROM_SIZE = 256;
const int EEPROM_DRIVES = 0x00;
const int EEPROM_SENSE_SW = EEPROM_DRIVES+FDD_NUM;
int EEPROM_idx;
void EEPROM_init() {
  //EEPROM clearing
       for (EEPROM_idx = 0 ; EEPROM_idx < EEPROM_SIZE ; EEPROM_idx++) {
          EEPROM.write(EEPROM_idx, 0);
       }
       //settings init
       for (EEPROM_idx = EEPROM_DRIVES ; EEPROM_idx < (EEPROM_DRIVES+FDD_NUM) ; EEPROM_idx++) {
        EEPROM.write(EEPROM_idx, uint8_t(EEPROM_idx-EEPROM_DRIVES));
       }
       //sense switches 0ff
       EEPROM.write(EEPROM_SENSE_SW, 0x00);
       //write signature to EEPROM
       //0xFE - 0x55
       EEPROM.write(0xFE, 0x55);
       //0xFF - 0xAA
       EEPROM.write(0xFF, 0xAA);
}
//------------------------------------------------------

//---------------------------------------------------
//DEBUG
uint16_t breakpoint = 0xFFFF;
boolean breakpointFlag = false;
char hex[2];
boolean CPM_logo = true;
//current state show
void state() {
  clrlin();
  Serial.print("A:");
  sprintf(hex, "%02X", _Regs[_Reg_A]);
  Serial.print(hex);
  Serial.print("   ");
  Serial.print("B:");
  sprintf(hex, "%02X", _Regs[_Reg_B]);
  Serial.print(hex);
  Serial.print("   ");
  Serial.print("C:");
  sprintf(hex, "%02X", _Regs[_Reg_C]);
  Serial.print(hex);
  Serial.print("   ");
  Serial.print("D:");
  sprintf(hex, "%02X", _Regs[_Reg_D]);
  Serial.print(hex);
  Serial.println("   ");
  clrlin();
  Serial.print("E:");
  sprintf(hex, "%02X", _Regs[_Reg_E]);
  Serial.print(hex);
  Serial.print("   ");
  Serial.print("H:");
  sprintf(hex, "%02X", _Regs[_Reg_H]);
  Serial.print(hex);
  Serial.print("   ");
  Serial.print("L:");
  sprintf(hex, "%02X", _Regs[_Reg_L]);
  Serial.print(hex);
  Serial.print("   ");
  Serial.print("F: ");
  if (_getFlags_S()==1) {
    Serial.print("S");
  }
  else {
    Serial.print(" ");
  }
  if (_getFlags_Z()==1) {
    Serial.print("Z");
  }
  else {
    Serial.print(" ");
  }
  if (_getFlags_A()==1) {
    Serial.print("A");
  }
  else {
    Serial.print(" ");
  }
  if (_getFlags_P()==1) {
    Serial.print("P");
  }
  else {
    Serial.print(" ");
  }
  if (_getFlags_C()==1) {
    Serial.print("C");
  }
  else {
    Serial.print(" ");
  }
  Serial.println("   ");
  clrlin();
  Serial.print(F("PC:"));
  sprintf(hex, "%02X", highByte(_PC));
  Serial.print(hex);
  sprintf(hex, "%02X", lowByte(_PC));
  Serial.print(hex);
  Serial.print("   ");
  Serial.print(F("SP:"));
  sprintf(hex, "%02X", highByte(_SP));
  Serial.print(hex);
  sprintf(hex, "%02X", lowByte(_SP));
  Serial.println(hex);
  clrlin();
  Serial.print(F("CMD: "));
  sprintf(hex, "%02X", _IR);
  Serial.print(hex);
  Serial.println("");
  //MMU map
  clrlin();
  Serial.print(F("MMU: "));
  for(int i=0;i<MMU_BLOCKS_NUM;i++) {
    Serial.print(MMU_MAP[i],DEC);
  }
  Serial.println("");
}

#include "BIOS.h"
#include "i8080_fns.h"

//memory check
uint32_t mem_test(boolean brk)
{
  uint32_t i;
  uint16_t j;
  uint32_t res;
  res = 0x10000L;
  //RAM write
  j = 0;
  for (i = 0; i <= 0xFFFF; i++) {
    _AB = i;
    _DB = pgm_read_byte_near(memtest_table + j);
    _WRMEM();
    if ((i % 8192) == 0) {
      Serial.print(".");
    }
    j++;
    if (j == MEMTEST_TABLE_SIZE) {
      j = 0;
    }
    if (brk && con_ready()) {
      con_read();
      return 0xFFFFF;//break
    }
  }
  //RAM read
  j = 0;
  for (i = 0; i <= 0xFFFF; i++) {
    if ((i % 8192) == 0) {
      Serial.print(".");
    }
    _AB = i;
    _RDMEM();
    if (_DB != pgm_read_byte_near(memtest_table + j)) {
      if (res>i) {
        res = i;
      }
    }
    j++;
    if (j == MEMTEST_TABLE_SIZE) {
      j = 0;
    }
        if (brk && con_ready()) {
          con_read();
          return 0xFFFFF;//break
    }
  }
  //RAM write (inverse)
  j = 0;
  for (i = 0; i <= 0xFFFF ; i++) {
    _AB = i;
    _DB = uint8_t(~(pgm_read_byte_near(memtest_table + j)));
    _WRMEM();
    if ((i % 8192) == 0) {
      Serial.print(".");
    }
    j++;
    if (j == MEMTEST_TABLE_SIZE) {
      j = 0;
    }
    if (brk && con_ready()) {
      con_read();
      return 0xFFFFF;//break
    }
  }
  //RAM read (inverse)
  j = 0;
  for (i = 0; i <= 0xFFFF; i++) {
    if ((i % 8192) == 0) {
      Serial.print(".");
    }
    _AB = i;
    _RDMEM();
    if (_DB != uint8_t(~(pgm_read_byte_near(memtest_table + j)))) {
      if (res>i) {
        res = i;
      }
    }
    j++;
    if (j == MEMTEST_TABLE_SIZE) {
      j = 0;
    }
    if (brk && con_ready()) {
      con_read();
      return 0xFFFFF;//break
    }
  }
  return res;
}


void call(word addr)
{
  byte cmd;
  bool exe_flag;
  exitFlag = false;
  _PC = addr;
  do
  {
    _AB = _PC;
    if (_AB ==  breakpoint) {
      DEBUG = true;
    }
    if (exitFlag) { break; } //go to monitor
    #include "BIOS_int.h"
    _RDMEM();//(AB) -> INSTR  instruction fetch
    _IR = _DB;
    #include "debug.h" 
    ((CmdFunction) pgm_read_word (&doCmdArray [_IR])) (); //decode
  } while (true);
  if (MEM_ERR) {
    MEM_ERR = false;
    clrscr();
    Serial.println("");
    Serial.println(F("MEMORY ERROR!"));
  }
}

void setup() {
  uint32_t i;
  uint16_t j;
  uint8_t k;
  uint32_t _cardsize;
  uint8_t res;
  bool RAMTestPass = true;
  uint32_t start_time;
  int CHECKED_BANKS;
  // start serial port at 9600 bps
  Serial.begin(9600);
  while (!Serial) {
    ; //wait for serial port to connect. Needed for native USB port only
  }
  delay(1000);
  clrscr();
  //pins setup
  pinMode(LED_pin, OUTPUT);
  digitalWrite(LED_pin, LOW);
  pinMode(IN_pin, INPUT);
  pinMode(OUT_pin, OUTPUT);
  digitalWrite(OUT_pin, LOW);
  //Timer1 setup
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 312;//50 Hz
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);//prescaler 1024
  TIMSK1 |= (1 << OCIE1A);//int enable
  //TIMSK1 = 0;//int disable
  sei();
  //logo
  color(2);
  Serial.println(F("***************************************"));
  Serial.print(F("*     CP/M for Arduino Nano V")); 
  Serial.print(VER_MAJOR);
  Serial.print(".");
  Serial.print(VER_MINOR);
  Serial.println(F("      *"));
  Serial.println(F("* (C) 2017 Alexey V.Voronin @ FoxyLab *"));
  Serial.println(F("*      https://acdc.foxylab.com       *"));
  Serial.println(F("***************************************"));
  Serial.println("");
/*
asm (
  "1:         \n"
  "nop        \n" //repeating code goes here
  );
  Serial.print("T");
  delay(100);
asm (
  "jmp 1b        \n"
  );
*/  

  
  //EEPROM checking
  if ((EEPROM.read(0xFE) != 0x55) || (EEPROM.read(0xFF) != 0xAA)) {
     //EEPROM init
     EEPROM_init();
  }
  //disks mount
  for (k=0; k<FDD_NUM; k++) {
    SD_FDD_OFFSET[k] = SD_DISKS_OFFSET + EEPROM.read(k)*SD_DISK_SIZE; 
  }  
  //sense switch
  SENSE_SW = EEPROM.read(EEPROM_SENSE_SW);
  //flush serial buffer
  con_flush();
  //MMU init
  for (i = 0; i < MMU_BLOCKS_NUM; i++) {
    MMU_MAP[i] = 0;
  }
  MMU_BLOCK_SEL_REG = 0;
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
    card.init(SPI_FULL_SPEED, SS_SD_pin);
    _cardsize = card.cardSize();
    if (_cardsize != 0) {
      Serial.println(F("CARD SIZE: "));
      Serial.print(_cardsize);
      Serial.println(F(" SECTORS"));
    }
    else {
      delay(250);
    }
  } while (_cardsize == 0);

  //SD RAM AREA CLEARING
  Serial.println(F("SD RAM AREA CLEARING..."));
  uint8_t LRC;
  uint32_t blk;
  uint32_t blk_end;
  blk = SD_MEM_OFFSET;
  blk_end = blk + ( MEM_SIZE*1024U / CACHE_LINE_SIZE ) + 2;
  do {
    LRC = 0;//LRC reset
    for(j=0;j<CACHE_LINE_SIZE;j++) {
      _buffer[j] = 0;
      LRC = 0 ^ LRC;//LRC calculation
    }
    _buffer[CACHE_LINE_SIZE] = LRC;//LRC add
    res = writeSD(blk);
    blk++;
  } while (blk < blk_end);

  Serial.println(F("SELECT BANK(S) FOR TEST: "));
  Serial.println(F("[0] - BANK 0, [1] - ALL BANKS"));
  RAM_TEST_MODE = 0xFF;
  start_time = millis();
  do {
    if (con_ready()) {
      inChar = con_read();
      switch (inChar) {
        case '0': RAM_TEST_MODE = 0;
          break;
        case '1': RAM_TEST_MODE = 1;
          break;
      }
    }
  } while ((RAM_TEST_MODE == 0xFF) && ((millis() - start_time) < SET_PAUSE));
  if (RAM_TEST_MODE == 0xFF) {
    RAM_TEST_MODE = 0x0;//Bank 0 check default
  }
  switch (RAM_TEST_MODE) {
    case 0: Serial.println(F(">>> BANK 0"));  
            CHECKED_BANKS = 1;
            break;
    case 1: Serial.println(F(">>> ALL BANKS"));
            CHECKED_BANKS = MMU_BANKS_NUM;
            break;
  }
  //MEMORY SPEED TEST
  /*
  uint32_t xxlen;
  uint16_t xxxx;
  uint32_t xxcnt;
  uint8_t xx;
  xxcnt = 0;//counter
  xxxx = 0xABCD;//seed
  xxlen = 20000;//reads number
  Serial.println(F("MEMORY SPEED TEST"));
  Serial.println(F("RND: START"));
  do {
    xxxx ^= xxxx << 2;
    xxxx ^= xxxx >> 7;
    xxxx ^= xxxx << 7;
    _AB = xxxx;
    _RDMEM();
    xx = _DB;
    xxcnt++;
  } while (xxcnt < xxlen);
  Serial.println(F("RND: STOP"));
  Serial.println(xxlen);//reads number
  xxxx = 0;
  xxlen = 100000;
  Serial.println(F("SEQ: START"));
  do {
    _AB = xxxx;
    _RDMEM();
    xx = _DB;
    xxxx++;
    //Serial.println(rnd);
    xxcnt++;
  } while (xxcnt < xxlen);
  Serial.println(F("SEQ: STOP"));
  Serial.println(xxlen);//reads number
  */
  //RAM TEST
  Serial.println(F("RAM TEST..."));
  for(int bank=0;bank<CHECKED_BANKS;bank++) {
    Serial.print(F("BANK "));
    Serial.print(k, HEX);
    //bank set for all blocks
    for (int block=0;block<MMU_BLOCKS_NUM;block++) {
      bank_set(block,bank);
    }
    RAM_AVAIL = mem_test(false);
    if (RAM_AVAIL != 0x10000) {
      Serial.println("RAM CHECK ERROR!");
      while(1) { }
    }
    Serial.println("");
  }
  Serial.print(RAM_AVAIL, DEC);
  Serial.println(F(" BYTE(S) OF RAM ARE AVAILABLE"));
  //RAM clear
  Serial.println(F("RAM CLEARING..."));
  for (i = 0; i < RAM_AVAIL; i++) {
    _AB = i;
    _DB = 0;
    _WRMEM();
    if ((i % 2048) == 0) {
      Serial.print("#");
    }
  }
  color(9);
  //stack init
  _SP = SP_INIT;
  DEBUG = true;//debug on
  delay(1000);
  clrscr();//clear screen
  xy(MON_Y, 0);//cursor positioning
  Serial.print('>');
}

//reset function
void(* sys_reset) (void) = 0;

boolean INTR = false;

//Timer1 interrupt
ISR(TIMER1_COMPA_vect) {
  if ( INTR ){ return; }
  INTR = true;
  sei();
  //WRITE LED off
  if (LED_on) {
    LED_count--;
    if (LED_count == 0) {
      LED_on = false;
      fastDigitalWrite(LED_pin, LOW);//LED off
    }
  }
  INTR = false;
}

void loop() {
  uint32_t adr;
  uint8_t port;
  uint8_t dat;
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
      if (con_ready()) {
        inChar = con_read();
        //Serial.print(uint8_t(inChar),HEX);
        inChar = upCase(inChar);
        if (uint8_t(inChar) == BS_KEY) {
          //backspace
          if (mon_ptr > 0) {
            mon_ptr--;
            mon_buffer[mon_ptr] = '\0';
            Serial.write(inChar);
            Serial.write(" ");
            Serial.write(inChar);
          }
        }
        else {
          mon_buffer[mon_ptr] = inChar;
          mon_ptr++;
          Serial.write(inChar);
        }
      }
    } while ((inChar != '\r') && (inChar != '\n') && (mon_ptr < MON_BUFFER_SIZE));
    Serial.print('\n');
    mon_ptr = 0;
    #include "mon.h"//monitor command execution
    xy(MON_Y, 0);//cursor positioning
    Serial.print('>');
    clrend();//clear input line to end
  }
}




