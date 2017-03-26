/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/

//TO DO
//screen position check

#define CTRL_C 3

  if (DEBUG) {
  savecur();
  xy(0,0);
  clrlin();
  Serial.print("A:");
  Serial.print(_Regs[_Reg_A], HEX);
  Serial.print("   ");
  Serial.print("B:");
  Serial.print(_Regs[_Reg_B], HEX);
  Serial.print("   ");
  Serial.print("C:");
  Serial.print(_Regs[_Reg_C], HEX);
  Serial.print("   ");
  Serial.print("D:");
  Serial.print(_Regs[_Reg_D], HEX);
  Serial.println("   ");
  clrlin();
  Serial.print("E:");
  Serial.print(_Regs[_Reg_E], HEX);
  Serial.print("   ");
  Serial.print("H:");
  Serial.print(_Regs[_Reg_H], HEX);
  Serial.print("   ");
  Serial.print("L:");
  Serial.print(_Regs[_Reg_L], HEX);
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
  Serial.print(_PC, HEX);
  Serial.print("   ");
  Serial.print(F("SP:"));
  Serial.println(_SP, HEX);
  clrlin();
  Serial.print(F("CMD: "));
  Serial.println(cmd, HEX);
  clrlin();
  Serial.println(F("Cache:"));
  clrlin();
  for(i=0;i<CACHE_LINES_NUM;i++) {
    clrlin();
    Serial.println((cache_tag[i]-SD_MEM_OFFSET)*CACHE_LINE_SIZE,HEX);  
  }
  clrlin();
  Serial.println("");
  clrlin();
  Serial.println(F("Press SPACE to continue, CTRL-C to break"));
  clrlin();
  inChar = '\0';
  do
  {
    if (Serial.available() > 0) {
          inChar = Serial.read();
    }
  } while ((inChar != ' ') && (inChar != CTRL_C));
  loadcur();
  if (inChar == CTRL_C) {
    exitFlag = true;
  }
  }

  


