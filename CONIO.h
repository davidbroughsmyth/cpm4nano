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
