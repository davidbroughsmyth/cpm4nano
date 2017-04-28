/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/

volatile uint8_t _DB; //data bus buffer
volatile uint16_t _AB; //data bus buffer

volatile bool exitFlag = false;

const uint8_t MEM_SIZE = 64;//System RAM Size, KBytes
const uint16_t MEM_MAX = (MEM_SIZE-1)*1024U + 1023U;//maximum system RAM address
uint32_t RAM_AVAIL = 0x10000L;//available RAM Size, KBytes (64 KBytes maximum)
const uint8_t RAM_SIZE = 64;//RAM Size for CP/M, KBytes


//AUX
boolean LED_on = false;
uint8_t LED_count;
const uint8_t LED_delay = 3;
const uint8_t LED_pin = 9;//D9 pin - LED
const uint8_t IN_pin = 8;//D8 pin - IN
const uint8_t OUT_pin = 7;//D7 pin - OUT
const uint8_t IN_PORT = 0xF0;//IN port
const uint8_t OUT_PORT = 0xF1;//OUT port
//----------------------------------------------------
//ALTAIR
uint8_t SENSE_SW = 0x00;//Altair/IMSAI sense switch default off
const uint8_t SENSE_SW_PORT = 0xFF;//Altair/IMSAI sense switch port
