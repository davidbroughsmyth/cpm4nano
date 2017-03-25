/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/

const uint32_t SD_MEM_OFFSET = 0x000400;
uint8_t RAM_MODE = 0;
uint32_t cur_blk = 0xFFFFFFFF;

uint8_t _getMEM(uint16_t adr) {
  uint8_t x;
  uint32_t blk;
  uint32_t blk_tmp;
  uint16_t start_tmp;
  uint16_t i;
  uint16_t j;
  uint8_t sel_blk;
  uint8_t res;
  switch (RAM_MODE) {
    case 0: //SD Card
        /* Serial.write(27);       // ESC command
  Serial.print("[2J");    // clear screen command
  Serial.write(27);
  Serial.print("[H");     // cursor to home command
*/
        blk = adr / CACHE_LINE_SIZE;
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
          //Serial.println("Cache miss");
          sel_blk = CACHE_LINES_NUM-1;
          if (cache_tag[sel_blk] != 0xFFFFFFFF) 
          {            
            if (cache_tag[0] != 0xFFFFFFFF) {
            //line 0 -> SD
            //Serial.println("Ex");
              if (cache_dirty[0]) {
                for(i=0;i<CACHE_LINE_SIZE;i++) {
                  _buffer[i] = cache[cache_start[0]+i];
                }
                res = writeSD(cache_tag[0]);
              }
            }
            //Serial.println("Up");
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
          //Serial.println("Refresh");
          cache_tag[sel_blk] = blk;
          res = readSD(blk, 0);
          for(i=0;i<CACHE_LINE_SIZE;i++) {
               cache[cache_start[sel_blk]+i] = _buffer[i];
            }
          cache_dirty[sel_blk] = false;
        }
        else { //cache hit
          //Serial.println("Cache hit");
          if (sel_blk != CACHE_LINES_NUM-1) {
            //cache lines swap
            //Serial.println("Swap");
            start_tmp = cache_start[sel_blk+1];
            cache_start[sel_blk+1] = cache_start[sel_blk];
            cache_start[sel_blk] = start_tmp;
            blk_tmp = cache_tag[sel_blk+1];
            cache_tag[sel_blk+1] = cache_tag[sel_blk];
            cache_tag[sel_blk] = blk_tmp;
            sel_blk++;
          }
        }
        adr = adr % CACHE_LINE_SIZE;
        //Serial.println("Read");
        x = cache[cache_start[sel_blk] + adr];//read from cache
        /*
        for(i=0;i<CACHE_LINES_NUM;i++) {
          for(j=0;j<CACHE_LINE_SIZE;j++) {
            //Serial.print(cache[cache_start[i] + j],HEX);  
          }
          //Serial.println("");
        }
  for(i=0;i<CACHE_LINES_NUM;i++) {
    //Serial.println(cache_start[i],HEX);  
  }*/
        break;
    case 1: //SPI SRAM

        break;
    case 2: //on-board RAM
        
        break;
  }
  return x;
}

void _setMEM(uint16_t adr, uint8_t  x) {
  uint32_t blk;
  uint32_t blk_tmp;
  uint16_t start_tmp;
  uint16_t i;
  uint16_t j;
  uint8_t sel_blk;
  uint8_t res;
  switch (RAM_MODE) {
    case 0: //SD Card
       /*
        Serial.write(27);       // ESC command
  Serial.print("[2J");    // clear screen command
  Serial.write(27);
  Serial.print("[H");     // cursor to home command
*/
        blk = adr / CACHE_LINE_SIZE;
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
          //Serial.println("Cache miss");
          sel_blk = CACHE_LINES_NUM-1;
          if (cache_tag[sel_blk] != 0xFFFFFFFF) 
          {            
            if (cache_tag[0] != 0xFFFFFFFF) {
            //line 0 -> SD
            //Serial.println("Ex");
              if (cache_dirty[0]) {
                for(i=0;i<CACHE_LINE_SIZE;i++) {
                _buffer[i] = cache[cache_start[0]+i];
                }
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
          //Serial.println("Refresh");
          cache_tag[sel_blk] = blk;
          res = readSD(blk, 0);
          for(i=0;i<CACHE_LINE_SIZE;i++) {
               cache[cache_start[sel_blk]+i] = _buffer[i];
            }
          cache_dirty[sel_blk] = false;
        }        
        else { //cache hit
          //Serial.println("Cache hit");
          if (sel_blk != CACHE_LINES_NUM-1) {
            //Serial.println("Swap");
            start_tmp = cache_start[sel_blk+1];
            cache_start[sel_blk+1] = cache_start[sel_blk];
            cache_start[sel_blk] = start_tmp;
            blk_tmp = cache_tag[sel_blk+1];
            cache_tag[sel_blk+1] = cache_tag[sel_blk];
            cache_tag[sel_blk] = blk_tmp;
            sel_blk++;
          }
        }
        adr = adr % CACHE_LINE_SIZE;
        //Serial.println("write");
        cache[cache_start[sel_blk] + adr] = x;//cache update
        cache_dirty[sel_blk] = true;
        /*
        for(i=0;i<CACHE_LINES_NUM;i++) {
          for(j=0;j<CACHE_LINE_SIZE;j++) {
            //Serial.print(cache[cache_start[i] + j],HEX);  
          }
          //Serial.println("");
        }
          for(i=0;i<CACHE_LINES_NUM;i++) {
    //Serial.println(cache_start[i],HEX);  
  }*/
        break;
    case 1: //SPI SRAM

        break;
    case 2: //on-board RAM
        //
        break;
  }
}

