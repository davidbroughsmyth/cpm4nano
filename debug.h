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
  state();
  Serial.println(F("Press SPACE to continue, CTRL-C to break"));
  clrlin();
  inChar = '\0';
  do
  {
    if (con_ready()) {
          inChar = con_read();
    }
  } while ((inChar != ' ') && (inChar != CTRL_C_KEY));
  loadcur();
  if (inChar == CTRL_C_KEY) {
    exitFlag = true;
  }
  }

  


