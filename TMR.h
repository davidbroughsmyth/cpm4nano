/*  CPM4NANO - i8080 & CP/M emulator for Arduino Nano 3.0 
*   Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab 
*   Email:    support@foxylab.com
*   Website:  https://acdc.foxylab.com
*/

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
}
