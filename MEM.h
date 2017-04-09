/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/
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
  //RAM mode set
  //RAM_MODE = 1;//SPI RAM
  RAM_MODE = 0;//SD Card
  //RAM get/set
  switch (RAM_MODE) {
    case 0: //SD Card
        blk = _AB >> CACHE_LINE_POW; 
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
        break;
    case 1: //SPI SRAM
        //_DB = readSPIRAM(_AB);
        break;
    case 2: //on-board RAM
        
        break;
  }
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
  //RAM mode set
  //RAM_MODE = 1;//SPI RAM
  RAM_MODE = 0;//SD Card
  //RAM get/set
  switch (RAM_MODE) {
    case 0: //SD Card
        blk = _AB >> CACHE_LINE_POW; 
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
        break;
    case 1: //SPI SRAM
        //writeSPIRAM(_AB,_DB);
        break;
    case 2: //on-board RAM
        //
        break;
  }
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


