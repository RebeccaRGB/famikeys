#include <Arduino.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "famikeysconfig.h"
#include "famikeyslib.h"
#include "usbkeyslib.h"

static unsigned char inputLastState[FK_STATE_SIZE];
static unsigned char inputBuffer[FK_BUFFER_SIZE];
static unsigned char inputBufStart = 0;
static unsigned char inputBufEnd = 0;
static void inputBufWrite(unsigned char packet) {
  inputBuffer[inputBufEnd] = packet;
  inputBufEnd++;
  inputBufEnd &= (FK_BUFFER_SIZE - 1);
  if (inputBufEnd == inputBufStart) {
    // Buffer overflowed. Ignore input.
    inputBufEnd--;
    inputBufEnd &= (FK_BUFFER_SIZE - 1);
  }
}

void fk_start_input() {
  // Initialize buffer.
  unsigned char i;
  for (i = 0; i < FK_STATE_SIZE; i++) {
    inputLastState[i] = 0;
  }
  inputBufStart = 0;
  inputBufEnd = 0;
  // Initialize pins for input.
  FK_D1_DDR   &=~ FK_D1_MASK;  // input
  FK_D1_PORT  |=  FK_D1_MASK;  // pull up
  FK_D2_DDR   &=~ FK_D2_MASK;  // input
  FK_D2_PORT  |=  FK_D2_MASK;  // pull up
  FK_D3_DDR   &=~ FK_D3_MASK;  // input
  FK_D3_PORT  |=  FK_D3_MASK;  // pull up
  FK_D4_DDR   &=~ FK_D4_MASK;  // input
  FK_D4_PORT  |=  FK_D4_MASK;  // pull up
  FK_RST_DDR  |=  FK_RST_MASK; // output
  FK_RST_PORT &=~ FK_RST_MASK; // pull down
  FK_INC_DDR  |=  FK_INC_MASK; // output
  FK_INC_PORT &=~ FK_INC_MASK; // pull down
  FK_ENA_DDR  |=  FK_ENA_MASK; // output
  FK_ENA_PORT &=~ FK_ENA_MASK; // pull down
}

boolean fk_connected() {
  unsigned char i;
  unsigned char m;
  // Select last row, which has no keys.
  FK_ENA_PORT |=  FK_ENA_MASK; // enable high
  FK_INC_PORT &=~ FK_INC_MASK; // increment low
  FK_RST_PORT |=  FK_RST_MASK; // reset high
  _delay_us(50);
  FK_RST_PORT &=~ FK_RST_MASK; // reset low
  _delay_us(50);
  for (i = 0; i < FK_STATE_SIZE; i++) {
    FK_INC_PORT |=  FK_INC_MASK; // increment low to high - next column
    _delay_us(50);
    FK_INC_PORT &=~ FK_INC_MASK; // increment high to low - next row
    _delay_us(50);
  }
  // Expected result is all low.
  if (FK_D1_PIN & FK_D1_MASK) m |= 0x01; else m &=~ 0x01;
  if (FK_D2_PIN & FK_D2_MASK) m |= 0x02; else m &=~ 0x02;
  if (FK_D3_PIN & FK_D3_MASK) m |= 0x04; else m &=~ 0x04;
  if (FK_D4_PIN & FK_D4_MASK) m |= 0x08; else m &=~ 0x08;
  // Disable keyboard.
  FK_ENA_PORT &=~ FK_ENA_MASK; // enable low
  FK_RST_PORT &=~ FK_RST_MASK; // reset low
  _delay_us(50);
  // Expected result is all high.
  if (FK_D1_PIN & FK_D1_MASK) m |= 0x10; else m &=~ 0x10;
  if (FK_D2_PIN & FK_D2_MASK) m |= 0x20; else m &=~ 0x20;
  if (FK_D3_PIN & FK_D3_MASK) m |= 0x40; else m &=~ 0x40;
  if (FK_D4_PIN & FK_D4_MASK) m |= 0x80; else m &=~ 0x80;
  return (m == 0xF0);
}

void fk_read_state(unsigned char * state) {
  unsigned char i;
  FK_ENA_PORT |=  FK_ENA_MASK; // enable high
  FK_INC_PORT &=~ FK_INC_MASK; // increment low
  FK_RST_PORT |=  FK_RST_MASK; // reset high
  _delay_us(50);
  FK_RST_PORT &=~ FK_RST_MASK; // reset low
  _delay_us(50);
  for (i = 0; i < FK_STATE_SIZE; i++, state++) {
    if (FK_D1_PIN & FK_D1_MASK) *state |= 0x01; else *state &=~ 0x01;
    if (FK_D2_PIN & FK_D2_MASK) *state |= 0x02; else *state &=~ 0x02;
    if (FK_D3_PIN & FK_D3_MASK) *state |= 0x04; else *state &=~ 0x04;
    if (FK_D4_PIN & FK_D4_MASK) *state |= 0x08; else *state &=~ 0x08;
    FK_INC_PORT |=  FK_INC_MASK; // increment low to high - next column
    _delay_us(50);
    if (FK_D1_PIN & FK_D1_MASK) *state |= 0x10; else *state &=~ 0x10;
    if (FK_D2_PIN & FK_D2_MASK) *state |= 0x20; else *state &=~ 0x20;
    if (FK_D3_PIN & FK_D3_MASK) *state |= 0x40; else *state &=~ 0x40;
    if (FK_D4_PIN & FK_D4_MASK) *state |= 0x80; else *state &=~ 0x80;
    FK_INC_PORT &=~ FK_INC_MASK; // increment high to low - next row
    _delay_us(50);
  }
}

unsigned char fk_read_packet() {
  unsigned char packet;
  unsigned char nextState[FK_STATE_SIZE];
  unsigned char n, i, r, p, m;
  fk_read_state(nextState);
  for (n = 0, i = 0; i < FK_STATE_SIZE; i++) {
    r = inputLastState[i] &~ nextState[i];
    p = nextState[i] &~ inputLastState[i];
    inputLastState[i] = nextState[i];
    for (m = 1; m != 0; m <<= 1, n++) {
      if (r & m) inputBufWrite(n | FK_RELEASED);
      if (p & m) inputBufWrite(n | FK_PRESSED);
    }
  }
  if (inputBufStart == inputBufEnd) return -1;
  packet = inputBuffer[inputBufStart];
  inputBufStart++;
  inputBufStart &= (FK_BUFFER_SIZE - 1);
  return packet;
}

static volatile unsigned char outputState[FK_STATE_SIZE];
static volatile unsigned char outputRow = 0;
static volatile unsigned char outputColumn = 0;
static volatile unsigned char outputLastInc = 0;

void fk_start_output() {
  // Initialize buffer.
  unsigned char i;
  for (i = 0; i < FK_STATE_SIZE; i++) {
    outputState[i] = 0;
  }
  outputRow = 0;
  outputColumn = 0;
  outputLastInc = 0;
  // Initialize pins for output.
  FK_D1_DDR   |=  FK_D1_MASK;  // output
  FK_D1_PORT  &=~ FK_D1_MASK;  // pull down
  FK_D2_DDR   |=  FK_D2_MASK;  // output
  FK_D2_PORT  &=~ FK_D2_MASK;  // pull down
  FK_D3_DDR   |=  FK_D3_MASK;  // output
  FK_D3_PORT  &=~ FK_D3_MASK;  // pull down
  FK_D4_DDR   |=  FK_D4_MASK;  // output
  FK_D4_PORT  &=~ FK_D4_MASK;  // pull down
  FK_RST_DDR  &=~ FK_RST_MASK; // input
  FK_RST_PORT &=~ FK_RST_MASK; // float
  FK_INC_DDR  &=~ FK_INC_MASK; // input
  FK_INC_PORT &=~ FK_INC_MASK; // float
  FK_ENA_DDR  &=~ FK_ENA_MASK; // input
  FK_ENA_PORT &=~ FK_ENA_MASK; // float
  // Set up pin change interrupts.
#ifdef FK_PCICR
  PCICR  |= FK_PCICR;
#endif
#ifdef FK_PCMSK0
  PCMSK0 |= FK_PCMSK0;
#endif
#ifdef FK_PCMSK1
  PCMSK1 |= FK_PCMSK1;
#endif
#ifdef FK_PCMSK2
  PCMSK2 |= FK_PCMSK2;
#endif
}

void fk_write_state(unsigned char * state) {
  unsigned char i;
  for (i = 0; i < FK_STATE_SIZE; i++) {
    outputState[i] = *state++;
  }
  fk_flush();
}

void fk_write_packet(unsigned char packet) {
  unsigned char m = 1 << (packet & 0x07);
  unsigned char i = (packet >> 3) & 0x0F;
  if (i < FK_STATE_SIZE) {
    if (packet & 0x80) {
      outputState[i] |= m;
    } else {
      outputState[i] &=~ m;
    }
  }
  fk_flush();
}

void fk_delay(unsigned long ms) {
  unsigned long time = millis();
  while ((millis() - time) < ms) {
  	fk_flush();
  }
}

void fk_flush() {
  noInterrupts();
  unsigned char rst = FK_RST_PIN & FK_RST_MASK;
  unsigned char inc = FK_INC_PIN & FK_INC_MASK;
  unsigned char ena = FK_ENA_PIN & FK_ENA_MASK;
  if (rst) {
    outputRow = 0;
    outputColumn = 0;
    outputLastInc = 0;
  }
  // increment low to high - next column
  if (!outputLastInc && inc) {
    outputColumn = 1;
    outputLastInc = inc;
  }
  // increment high to low - next row
  if (outputLastInc && !inc) {
    outputRow++;
    if (outputRow >= 10) {
      outputRow = 0;
    }
    outputColumn = 0;
    outputLastInc = inc;
  }
  if (!ena) {
    FK_D1_PORT |= FK_D1_MASK;
    FK_D2_PORT |= FK_D2_MASK;
    FK_D3_PORT |= FK_D3_MASK;
    FK_D4_PORT |= FK_D4_MASK;
  } else if (outputRow >= FK_STATE_SIZE) {
    FK_D1_PORT &=~ FK_D1_MASK;
    FK_D2_PORT &=~ FK_D2_MASK;
    FK_D3_PORT &=~ FK_D3_MASK;
    FK_D4_PORT &=~ FK_D4_MASK;
  } else {
    unsigned char bit = outputState[outputRow];
    if (outputColumn) {
      if (bit & 0x10) FK_D1_PORT |= FK_D1_MASK; else FK_D1_PORT &=~ FK_D1_MASK;
      if (bit & 0x20) FK_D2_PORT |= FK_D2_MASK; else FK_D2_PORT &=~ FK_D2_MASK;
      if (bit & 0x40) FK_D3_PORT |= FK_D3_MASK; else FK_D3_PORT &=~ FK_D3_MASK;
      if (bit & 0x80) FK_D4_PORT |= FK_D4_MASK; else FK_D4_PORT &=~ FK_D4_MASK;
    } else {
      if (bit & 0x01) FK_D1_PORT |= FK_D1_MASK; else FK_D1_PORT &=~ FK_D1_MASK;
      if (bit & 0x02) FK_D2_PORT |= FK_D2_MASK; else FK_D2_PORT &=~ FK_D2_MASK;
      if (bit & 0x04) FK_D3_PORT |= FK_D3_MASK; else FK_D3_PORT &=~ FK_D3_MASK;
      if (bit & 0x08) FK_D4_PORT |= FK_D4_MASK; else FK_D4_PORT &=~ FK_D4_MASK;
    }
  }
  interrupts();
}

#ifdef FK_PCMSK0
ISR(PCINT0_vect) { fk_flush(); }
#endif
#ifdef FK_PCMSK1
ISR(PCINT1_vect) { fk_flush(); }
#endif
#ifdef FK_PCMSK2
ISR(PCINT2_vect) { fk_flush(); }
#endif

const char * fk_key_name(unsigned char key) {
  switch (key & 0x7F) {
    case 0x00: return "F8";
    case 0x01: return "Return";
    case 0x02: return "[";
    case 0x03: return "]";
    case 0x04: return "Kana";
    case 0x05: return "R Shift";
    case 0x06: return "Yen";
    case 0x07: return "Stop";
    case 0x08: return "F7";
    case 0x09: return "@";
    case 0x0A: return ":*";
    case 0x0B: return ";+";
    case 0x0C: return "Coda";
    case 0x0D: return "/";
    case 0x0E: return "-";
    case 0x0F: return "^";
    case 0x10: return "F6";
    case 0x11: return "O";
    case 0x12: return "L";
    case 0x13: return "K";
    case 0x14: return ".";
    case 0x15: return ",";
    case 0x16: return "P";
    case 0x17: return "0";
    case 0x18: return "F5";
    case 0x19: return "I";
    case 0x1A: return "U";
    case 0x1B: return "J";
    case 0x1C: return "M";
    case 0x1D: return "N";
    case 0x1E: return "9";
    case 0x1F: return "8";
    case 0x20: return "F4";
    case 0x21: return "Y";
    case 0x22: return "G";
    case 0x23: return "H";
    case 0x24: return "B";
    case 0x25: return "V";
    case 0x26: return "7";
    case 0x27: return "6";
    case 0x28: return "F3";
    case 0x29: return "T";
    case 0x2A: return "R";
    case 0x2B: return "D";
    case 0x2C: return "F";
    case 0x2D: return "C";
    case 0x2E: return "5";
    case 0x2F: return "4";
    case 0x30: return "F2";
    case 0x31: return "W";
    case 0x32: return "S";
    case 0x33: return "A";
    case 0x34: return "X";
    case 0x35: return "Z";
    case 0x36: return "E";
    case 0x37: return "3";
    case 0x38: return "F1";
    case 0x39: return "Esc";
    case 0x3A: return "Q";
    case 0x3B: return "Ctr";
    case 0x3C: return "L Shift";
    case 0x3D: return "Grph";
    case 0x3E: return "1";
    case 0x3F: return "2";
    case 0x40: return "Clr/Home";
    case 0x41: return "Up";
    case 0x42: return "Right";
    case 0x43: return "Left";
    case 0x44: return "Down";
    case 0x45: return "Space";
    case 0x46: return "Del";
    case 0x47: return "Ins";
    default:   return "???";
  }
}

static const char PROGMEM keysAscii[] = {
  0x18, 0x0A, '[',  ']',  0x10, 0,    '\\', 0x03,
  0x17, '@',  ':',  ';',  '_',  '/',  '-',  '^',
  0x16, 'o',  'l',  'k',  '.',  ',',  'p',  '0',
  0x15, 'i',  'u',  'j',  'm',  'n',  '9',  '8',
  0x14, 'y',  'g',  'h',  'b',  'v',  '7',  '6',
  0x13, 't',  'r',  'd',  'f',  'c',  '5',  '4',
  0x12, 'w',  's',  'a',  'x',  'z',  'e',  '3',
  0x11, 0x1B, 'q',  0,    0,    0,    '1',  '2',
  0x0C, 0x1E, 0x1D, 0x1C, 0x1F, ' ',  0x08, 0x09,
};

static const char PROGMEM keysAsciiShift[] = {
  0x18, 0x0A, '{',  '}',  0x10, 0,    '|',  0x03,
  0x17, '`',  '*',  '+',  0x7F, '?',  '=',  '~',
  0x16, 'O',  'L',  'K',  '>',  '<',  'P',  ' ',
  0x15, 'I',  'U',  'J',  'M',  'N',  ')',  '(',
  0x14, 'Y',  'G',  'H',  'B',  'V',  '\'', '&',
  0x13, 'T',  'R',  'D',  'F',  'C',  '%',  '$',
  0x12, 'W',  'S',  'A',  'X',  'Z',  'E',  '#',
  0x11, 0x1B, 'Q',  0,    0,    0,    '!',  '"',
  0x0C, 0x1E, 0x1D, 0x1C, 0x1F, ' ',  0x08, 0x09,
};

char fk_key_to_ascii(unsigned char key, boolean shift) {
  if ((key &= 0x7F) < 0x48) {
    if (shift) return pgm_read_byte(&keysAsciiShift[key]);
    else       return pgm_read_byte(&keysAscii     [key]);
  } else {
    return 0;
  }
}

static const unsigned char PROGMEM keysUsb[] = {
  USBK_F8,         // F8
  USBK_RETURN,     // Return
  USBK_LT_BRACKET, // [
  USBK_RT_BRACKET, // ]
  USBK_RT_ALT,     // Kana
  USBK_RT_SHIFT,   // Rt Shift
  USBK_BACKSLASH,  // Yen
  USBK_PAUSE,      // Stop
  USBK_F7,         // F7
  USBK_TILDE,      // @
  USBK_QUOTE,      // : *
  USBK_SEMICOLON,  // ; +
  USBK_102ND,      // Coda
  USBK_SLASH,      // / ?
  USBK_MINUS,      // -
  USBK_EQUAL,      // ^
  USBK_F6, USBK_O, USBK_L, USBK_K, USBK_PERIOD, USBK_COMMA, USBK_P, USBK_0,
  USBK_F5, USBK_I, USBK_U, USBK_J, USBK_M,      USBK_N,     USBK_9, USBK_8,
  USBK_F4, USBK_Y, USBK_G, USBK_H, USBK_B,      USBK_V,     USBK_7, USBK_6,
  USBK_F3, USBK_T, USBK_R, USBK_D, USBK_F,      USBK_C,     USBK_5, USBK_4,
  USBK_F2, USBK_W, USBK_S, USBK_A, USBK_X,      USBK_Z,     USBK_E, USBK_3,
  USBK_F1,         // F1
  USBK_ESC,        // Esc
  USBK_Q,          // Q
  USBK_LT_CTRL,    // Ctr
  USBK_LT_SHIFT,   // Lt Shift
  USBK_LT_ALT,     // Grph
  USBK_1,          // 1 !
  USBK_2,          // 2 "
  USBK_HOME,       // Clr/Home
  USBK_UP,         // Up Arrow
  USBK_RIGHT,      // Rt Arrow
  USBK_LEFT,       // Lt Arrow
  USBK_DOWN,       // Dn Arrow
  USBK_SPACE,      // Space
  USBK_BACKSPACE,  // Del
  USBK_TAB,        // Ins
};

static const unsigned char PROGMEM keysUsbAlt[] = {
  USBK_POWER,      // F8
  USBK_NUM_ENTER,  // Return
  0x81,            // [        -> Vol-
  0x80,            // ]        -> Vol+
  USBK_RT_ALT,     // Kana
  USBK_RT_SHIFT,   // Rt Shift
  0x7F,            // Yen      -> Mute
  USBK_BACKSPACE,  // Stop
  USBK_PAUSE,      // F7
  USBK_NUM_SLASH,  // @
  USBK_NUM_TIMES,  // : *
  USBK_NUM_PLUS,   // ; +
  USBK_POWER,      // Coda
  USBK_MENU,       // / ?
  USBK_NUM_EQUAL,  // -
  0xEC,            // ^        -> Eject
  USBK_SCRLK,      // F6
  USBK_NUM_6,      // O
  USBK_NUM_3,      // L
  USBK_NUM_2,      // K
  USBK_NUM_PERIOD, // . >
  0x85,            // , <      -> Num ,
  USBK_NUM_MINUS,  // P
  USBK_NUM_0,      // 0
  USBK_SYSRQ,      // F5
  USBK_NUM_5,      // I
  USBK_NUM_4,      // U
  USBK_NUM_1,      // J
  USBK_NUM_0,      // M
  USBK_PAUSE,      // N
  USBK_NUM_9,      // 9 )
  USBK_NUM_8,      // 8 (
  USBK_F12,        // F4
  USBK_SYSRQ,      // Y
  USBK_F8,         // G
  USBK_SCRLK,      // H
  USBK_F12,        // B
  USBK_F11,        // V
  USBK_NUM_7,      // 7 '
  USBK_NUM_6,      // 6 &
  USBK_F11,        // F3
  USBK_F4,         // T
  USBK_F3,         // R
  USBK_F6,         // D
  USBK_F7,         // F
  USBK_F10,        // C
  USBK_NUM_5,      // 5 %
  USBK_NUM_4,      // 4 $
  USBK_F10,        // F2
  USBK_F1,         // W
  USBK_F5,         // S
  USBK_CAPS_LOCK,  // A
  USBK_F9,         // X
  USBK_102ND,      // Z
  USBK_F2,         // E
  USBK_NUM_3,      // 3 #
  USBK_F9,         // F1
  USBK_ESC,        // Esc
  USBK_TAB,        // Q
  USBK_LT_CTRL,    // Ctr
  USBK_LT_SHIFT,   // Lt Shift
  USBK_LT_ALT,     // Grph
  USBK_NUM_1,      // 1 !
  USBK_NUM_2,      // 2 "
  USBK_NUM_CLEAR,  // Clr/Home
  USBK_PGUP,       // Up Arrow
  USBK_END,        // Rt Arrow
  USBK_HOME,       // Lt Arrow
  USBK_PGDN,       // Dn Arrow
  USBK_SPACE,      // Space
  USBK_DEL,        // Del
  USBK_INS,        // Ins
};

unsigned char fk_key_to_usb(unsigned char key, boolean alt) {
  if ((key &= 0x7F) < 0x48) {
    if (alt) {
      return pgm_read_byte(&keysUsbAlt[key]);
    } else {
      return pgm_read_byte(&keysUsb[key]);
    }
  } else {
    return 0;
  }
}

static const unsigned char PROGMEM keyFromUsb[] = {
  -1, -1, -1, -1,
  FK_A, FK_B, FK_C, FK_D, FK_E, FK_F, FK_G, FK_H, FK_I, FK_J,
  FK_K, FK_L, FK_M, FK_N, FK_O, FK_P, FK_Q, FK_R, FK_S, FK_T,
  FK_U, FK_V, FK_W, FK_X, FK_Y, FK_Z, FK_1, FK_2, FK_3, FK_4,
  FK_5, FK_6, FK_7, FK_8, FK_9, FK_0, FK_RETURN, FK_ESC,

  FK_DEL,            // Backspace
  FK_INS,            // Tab
  FK_SPACE,          // Space
  FK_HYPHEN,         // - _
  FK_CARET,          // = +
  FK_LT_BRACKET,     // [ {
  FK_RT_BRACKET,     // ] }
  FK_YEN,            // \ |
  FK_CODA,           // ~ #
  FK_SEMICOLON,      // ; :
  FK_COLON,          // ' "
  FK_AT,             // ` ~
  FK_COMMA,          // , <
  FK_PERIOD,         // . >
  FK_SLASH,          // / ?
  FK_CTR,            // Caps Lock

  FK_F1, FK_F2, FK_F3, FK_F4, FK_F5, FK_F6, FK_F7, FK_F8,

  FK_CLR_HOME,       // F9
  FK_STOP,           // F10
  FK_YEN,            // F11
  FK_AT,             // F12
  FK_CODA,           // PrtSc
  FK_KANA,           // ScrLk
  FK_STOP,           // Pause

  FK_INS,            // Ins
  FK_CLR_HOME,       // Home
  FK_KANA,           // PgUp
  FK_DEL,            // Del
  FK_STOP,           // End
  FK_CODA,           // PgDn

  FK_RT_ARROW, FK_LT_ARROW, FK_DN_ARROW, FK_UP_ARROW,

  FK_ESC,            // Num Lock
  FK_SLASH,          // Num /
  FK_COLON|0x80,     // Num *
  FK_HYPHEN,         // Num -
  FK_SEMICOLON|0x80, // Num +
  FK_RETURN,         // Num Enter

  FK_1, FK_2, FK_3, FK_4, FK_5, FK_6, FK_7, FK_8, FK_9, FK_0,

  FK_PERIOD,         // Num .
  FK_CODA,           // 102nd
  FK_KANA,           // Menu
  -1,                // Power
  FK_HYPHEN|0x80,    // Num =
};

unsigned char fk_key_from_usb(unsigned char usb) {
  if (usb < 0x68) return pgm_read_byte(&keyFromUsb[usb]);
  if (usb == 0x82) return FK_CTR;
  if (usb == 0x85) return FK_COMMA;
  return -1;
}
