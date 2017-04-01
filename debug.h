/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/

//TO DO
//screen position check

  if (DEBUG) {
  savecur();
  xy(0,0);
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
  sprintf(hex, "%02X", cmd);
  Serial.print(hex);
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
  } while ((inChar != ' ') && (inChar != CTRL_C_KEY));
  loadcur();
  if (inChar == CTRL_C_KEY) {
    exitFlag = true;
  }
  }

  


