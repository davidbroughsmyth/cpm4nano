/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/

//FDD controller ports
const uint8_t FDD_BASE = 0xE0;//FDD base address
const uint8_t FDD_PORT_CMD = FDD_BASE + 0; //status/command
const uint8_t FDD_PORT_TRK = FDD_BASE + 1; //track
const uint8_t FDD_PORT_SEC = FDD_BASE + 2; //sector
const uint8_t FDD_PORT_DRV = FDD_BASE + 3; //drive select

//DMA controller ports
const uint8_t FDD_PORT_DMA_ADDR_LO = FDD_BASE + 4; //DMA address low byte
const uint8_t FDD_PORT_DMA_ADDR_HI = FDD_BASE + 5; //DMA address high byte

//FDD commands codes
const uint8_t FDD_RD_CMD = 0x00; //read sector command
const uint8_t FDD_WRT_CMD = 0x01; //write sector command

//FDD registers
uint8_t FDD_REG_SEC = 1; //sector register
uint8_t FDD_REG_TRK = 0; //track register
uint8_t FDD_REG_DRV = 0; //drive register
boolean FDD_REG_STATUS = false; //true - O.K., false - ERROR
uint16_t FDD_REG_DMA = 0; //DMA address register
