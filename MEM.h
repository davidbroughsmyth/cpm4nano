/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/

extern boolean con_ready();
extern char con_read();

const uint32_t SD_MEM_OFFSET = 0x070000;//memory offset in SD-card
boolean MEM_ERR = false;//LRC memory error flag

const static uint8_t PROGMEM memtest_table[] = {
  0x3D, 0x55, 0x5F, 0x15, 0x23, 0x47, 0x1C, 0x31, 0x48, 0x60, 0x35, 0x11, 0x4F, 0x2F, 0x2E, 0x14, 0x20, 0x5B, 0x39, 0x26, 0x09, 0x61, 0x34, 0x30, 0x50, 0x2B, 0x4B, 0x0F, 0x63, 0x1F, 0x10, 0x1E, 0x36,
};
const uint16_t MEMTEST_TABLE_SIZE = 33;
uint8_t RAM_TEST_MODE;//memory check mpde

//CACHE
//constants
const uint16_t CACHE_LINE_SIZE = 64;//cache line size
const uint8_t CACHE_LINES_NUM = 8;//cache lines number
const uint16_t CACHE_SIZE = CACHE_LINES_NUM * CACHE_LINE_SIZE;//total cache size
const uint32_t CACHE_LINE_EMPTY = 0xFFFFFFFF;//empty cache line flag
//arrays
static uint8_t cache[CACHE_SIZE];//cache
uint32_t cache_tag[CACHE_LINES_NUM];//cache line tag (block #)
uint16_t cache_start[CACHE_LINES_NUM];//cache line start
boolean cache_dirty[CACHE_LINES_NUM];//cache line dirty flag

//MMU
//ports
const uint8_t MMU_BLOCK_SEL_PORT = 0xD0;//block select port
const uint8_t MMU_BANK_SEL_PORT = 0xD1;//bank select port
//registers
uint8_t MMU_BLOCK_SEL_REG = 0x00;//block select register
uint8_t MMU_BANK_SEL_REG = 0x00;//bank register
//constants
const uint8_t MMU_BANKS_NUM = 8;//banks number
const uint16_t MMU_BLOCK_SIZE = 4096;//4096 bytes - block size 
const uint8_t MMU_BLOCKS_NUM = 65536UL / MMU_BLOCK_SIZE;//blocks number
//map
uint8_t MMU_MAP[MMU_BLOCKS_NUM];//memory banking map
//set bank for block
void bank_set(uint8_t block, uint8_t bank)
{
  MMU_MAP[block] = bank;
}
//get bank for block
uint8_t bank_get(uint8_t block)
{
  return MMU_MAP[block];
}

//address <- _AB
//data -> _DB

void _RDMEM() {
  uint32_t blk;
  uint32_t blk_tmp;
  uint16_t start_tmp;
  uint16_t i;
  uint8_t sel_blk;
  uint8_t res;
  uint8_t LRC;
  if (_AB>MEM_MAX) {
    _DB = 0xFF;//not memory
    return;
  }
        blk = ((uint32_t)(_AB) + (uint32_t)((MMU_MAP[_AB / MMU_BLOCK_SIZE]) * 65536UL)) / CACHE_LINE_SIZE; 
        blk = blk +  SD_MEM_OFFSET;
        //Serial.println(blk, HEX);
        sel_blk = 0xff;
        i=0;
        do {
          if (blk == cache_tag[i]) {
            sel_blk = i;
          }
          i++;
        } while ((sel_blk == 0xff) && (i<CACHE_LINES_NUM)) ;
        if (sel_blk == 0xff) { //cache miss
          sel_blk = CACHE_LINES_NUM-1;
          if (cache_tag[sel_blk] != CACHE_LINE_EMPTY) 
          {            
            if (cache_tag[0] != CACHE_LINE_EMPTY) {
            //line 0 -> SD
              if (cache_dirty[0]) {
                LRC = 0;//LRC reset
                for(i=0;i<CACHE_LINE_SIZE;i++) {
                  _buffer[i] = cache[cache_start[0]+i];
                  LRC = _buffer[i] ^ LRC;//LRC calculation
                }
                _buffer[CACHE_LINE_SIZE] = LRC;//LRC write
                res = writeSD(cache_tag[0]);
              }
            }
            //move up
            start_tmp = cache_start[0];
            blk_tmp = cache_tag[0];
            for(i=1;i<CACHE_LINES_NUM;i++) {
              cache_start[i-1] = cache_start[i];
              cache_tag[i-1] = cache_tag[i];
            }
            cache_start[CACHE_LINES_NUM-1] = start_tmp;
            cache_tag[CACHE_LINES_NUM-1] = blk_tmp;
          }
          //read new line from SD
          cache_tag[sel_blk] = blk;
          res = readSD(blk, 0);
          LRC = 0;//LRC reset
          for(i=0;i<CACHE_LINE_SIZE;i++) {
               cache[cache_start[sel_blk]+i] = _buffer[i];
               LRC = _buffer[i] ^ LRC;//LRC calculation
            }
          if (_buffer[CACHE_LINE_SIZE] != LRC) {
            MEM_ERR = true;
            exitFlag = true;//quit to monitor
            _DB = 0x00;
            return;
          }
          cache_dirty[sel_blk] = false;
        }
        else { //cache hit
          if (sel_blk != CACHE_LINES_NUM-1) {
            //cache lines swap
            start_tmp = cache_start[sel_blk+1];
            cache_start[sel_blk+1] = cache_start[sel_blk];
            cache_start[sel_blk] = start_tmp;
            blk_tmp = cache_tag[sel_blk+1];
            cache_tag[sel_blk+1] = cache_tag[sel_blk];
            cache_tag[sel_blk] = blk_tmp;
            sel_blk++;
          }
        }
        _DB = cache[cache_start[sel_blk] + (_AB & (CACHE_LINE_SIZE - 1))];//read from cache
}

//address <- _AB
//data <- _DB
void _WRMEM() {
  uint32_t blk;
  uint32_t blk_tmp;
  uint16_t start_tmp;
  uint16_t i;
  uint8_t sel_blk;
  uint8_t res;
  uint8_t LRC;
  if (_AB>MEM_MAX) {
    return;
  }
        blk = ((uint32_t)(_AB) + (uint32_t)((MMU_MAP[_AB / MMU_BLOCK_SIZE]) * 65536UL)) / CACHE_LINE_SIZE; 
        blk = blk +  SD_MEM_OFFSET;
        sel_blk = 0xff;
        i=0;
        do {
          if (blk == cache_tag[i]) {
            sel_blk = i;
          }
          i++;
        } while ((sel_blk == 0xff) && (i<CACHE_LINES_NUM)) ;
        if (sel_blk == 0xff) { //cache miss
          sel_blk = CACHE_LINES_NUM-1;
          if (cache_tag[sel_blk] != CACHE_LINE_EMPTY) 
          {            
            if (cache_tag[0] != CACHE_LINE_EMPTY) {
            //line 0 -> SD
              if (cache_dirty[0]) {
                LRC = 0;//LRC reset
                for(i=0;i<CACHE_LINE_SIZE;i++) {
                  _buffer[i] = cache[cache_start[0]+i];
                  LRC = _buffer[i] ^ LRC;//LRC calculation
                }
                _buffer[CACHE_LINE_SIZE] = LRC;//LRC add
                res = writeSD(cache_tag[0]);
              }
            }
            //move up
            start_tmp = cache_start[0];
            blk_tmp = cache_tag[0];
            for(i=1;i<CACHE_LINES_NUM;i++) {
              cache_start[i-1] = cache_start[i];
              cache_tag[i-1] = cache_tag[i];
            }
            cache_start[CACHE_LINES_NUM-1] = start_tmp;
            cache_tag[CACHE_LINES_NUM-1] = blk_tmp;
          }
          //read new line from SD
          cache_tag[sel_blk] = blk;
          res = readSD(blk, 0);
          LRC = 0;//LRC reset
          for(i=0;i<CACHE_LINE_SIZE;i++) {
               cache[cache_start[sel_blk]+i] = _buffer[i];
               LRC = _buffer[i] ^ LRC;//LRC calculation
            }
          if (_buffer[CACHE_LINE_SIZE] != LRC) {
            MEM_ERR = true;
            exitFlag = true;//quit to monitor
            return;
          }
          cache_dirty[sel_blk] = false;
        }        
        else { //cache hit
          if (sel_blk != CACHE_LINES_NUM-1) {
            start_tmp = cache_start[sel_blk+1];
            cache_start[sel_blk+1] = cache_start[sel_blk];
            cache_start[sel_blk] = start_tmp;
            blk_tmp = cache_tag[sel_blk+1];
            cache_tag[sel_blk+1] = cache_tag[sel_blk];
            cache_tag[sel_blk] = blk_tmp;
            sel_blk++;
          }
        }
        cache[cache_start[sel_blk] + (_AB & (CACHE_LINE_SIZE - 1))] = _DB;//cache update
        cache_dirty[sel_blk] = true;
}

uint8_t _getMEM(uint16_t adr) {
  _AB = adr;
  _RDMEM();
  return _DB;
}

void _setMEM(uint16_t adr, uint8_t dat) {
  _AB = adr;
  _DB = dat;
  _WRMEM();
}

//MEMORY TEST
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
