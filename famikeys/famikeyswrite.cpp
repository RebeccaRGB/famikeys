#include <Arduino.h>
#include <avr/pgmspace.h>
#include "famikeyslib.h"
#include "famikeyswrite.h"

static unsigned char digit_value(unsigned char ch) {
  if (ch >= '0' && ch <= '9') return ch - '0';
  if (ch >= 'A' && ch <= 'Z') return ch - 'A' + 10;
  if (ch >= 'a' && ch <= 'z') return ch - 'a' + 10;
  return 0xFF;
}

void fk_write_hex(unsigned char * s) {
  boolean inValue = false;
  unsigned char value;
  unsigned char ch;
  while ((ch = *s)) {
    if ((ch = digit_value(ch)) < 16) {
      if (inValue) {
        inValue = false;
        fk_write_packet(value | ch);
        fk_delay(FK_WRITE_DELAY);
        value = 0;
      } else {
        inValue = true;
        value = ch << 4;
      }
    }
    s++;
  }
}

static const unsigned char PROGMEM keyFromAscii[] = {
  -1,             -1,                 -1,              FK_STOP,
  -1,             -1,                 -1,              -1,
  FK_DEL,         FK_INS,             FK_RETURN,       FK_RETURN,
  FK_CLR_HOME,    FK_RETURN,          -1,              -1,
  FK_KANA,        FK_F1,              FK_F2,           FK_F3,
  FK_F4,          FK_F5,              FK_F6,           FK_F7,
  FK_F8,          -1,                 -1,              FK_ESC,
  FK_LT_ARROW,    FK_RT_ARROW,        FK_UP_ARROW,     FK_DN_ARROW,
  FK_SPACE,       FK_1|0x80,          FK_2|0x80,       FK_3|0x80,
  FK_4|0x80,      FK_5|0x80,          FK_6|0x80,       FK_7|0x80,
  FK_8|0x80,      FK_9|0x80,          FK_COLON|0x80,   FK_SEMICOLON|0x80,
  FK_COMMA,       FK_HYPHEN,          FK_PERIOD,       FK_SLASH,
  FK_0,           FK_1,               FK_2,            FK_3,
  FK_4,           FK_5,               FK_6,            FK_7,
  FK_8,           FK_9,               FK_COLON,        FK_SEMICOLON,
  FK_COMMA|0x80,  FK_HYPHEN|0x80,     FK_PERIOD|0x80,  FK_SLASH|0x80,
  FK_AT,          FK_A|0x80,          FK_B|0x80,       FK_C|0x80,
  FK_D|0x80,      FK_E|0x80,          FK_F|0x80,       FK_G|0x80,
  FK_H|0x80,      FK_I|0x80,          FK_J|0x80,       FK_K|0x80,
  FK_L|0x80,      FK_M|0x80,          FK_N|0x80,       FK_O|0x80,
  FK_P|0x80,      FK_Q|0x80,          FK_R|0x80,       FK_S|0x80,
  FK_T|0x80,      FK_U|0x80,          FK_V|0x80,       FK_W|0x80,
  FK_X|0x80,      FK_Y|0x80,          FK_Z|0x80,       FK_LT_BRACKET,
  FK_YEN,         FK_RT_BRACKET,      FK_CARET,        FK_CODA,
  FK_AT|0x80,     FK_A,               FK_B,            FK_C,
  FK_D,           FK_E,               FK_F,            FK_G,
  FK_H,           FK_I,               FK_J,            FK_K,
  FK_L,           FK_M,               FK_N,            FK_O,
  FK_P,           FK_Q,               FK_R,            FK_S,
  FK_T,           FK_U,               FK_V,            FK_W,
  FK_X,           FK_Y,               FK_Z,            FK_LT_BRACKET|0x80,
  FK_YEN|0x80,    FK_RT_BRACKET|0x80, FK_CARET|0x80,   FK_CODA|0x80
};

void fk_write_ascii(unsigned char * s) {
  unsigned char shifted;
  unsigned char ch;
  while ((ch = *s)) {
    if (ch <= 0x7F) {
      ch = pgm_read_byte(&keyFromAscii[ch]);
      shifted = ch & 0x80;
      if ((ch &= 0x7F) < 0x48) {
        if (shifted) {
          fk_write_packet(FK_LT_SHIFT | FK_PRESSED);
          fk_delay(FK_WRITE_DELAY);
        }
        fk_write_packet(ch | FK_PRESSED);
        fk_delay(FK_WRITE_DELAY);
        fk_write_packet(ch | FK_RELEASED);
        fk_delay(FK_WRITE_DELAY);
        if (shifted) {
          fk_write_packet(FK_LT_SHIFT | FK_RELEASED);
          fk_delay(FK_WRITE_DELAY);
        }
      }
    }
    s++;
  }
}

static const unsigned char PROGMEM codedValues[] = {
  FK_SPACE,     FK_F1,         FK_F2,         FK_F3,
  FK_F4,        FK_F5,         FK_F6,         FK_F7,
  -1,           -1,            FK_COLON,      FK_SEMICOLON,
  FK_COMMA,     FK_HYPHEN,     FK_PERIOD,     FK_SLASH,
  FK_0,         FK_1,          FK_2,          FK_3,
  FK_4,         FK_5,          FK_6,          FK_7,
  FK_8,         FK_9,          FK_COLON,      FK_SEMICOLON,
  FK_LT_ARROW,  FK_STOP,       FK_RT_ARROW,   FK_F8,
  FK_AT,        FK_UP_ARROW,   FK_DN_ARROW,   FK_CTR,
  FK_DEL,       FK_ESC,        FK_LT_BRACKET, FK_GRPH,
  FK_CLR_HOME,  FK_INS,        FK_RT_BRACKET, FK_KANA,
  FK_LT_SHIFT,  FK_RT_SHIFT,   FK_CODA,       FK_AT,
  FK_LT_ARROW,  FK_RT_ARROW,   FK_RETURN,     FK_LT_SHIFT,
  FK_RT_SHIFT,  FK_UP_ARROW,   FK_DN_ARROW,   FK_SPACE,
  FK_CARET,     FK_YEN,        FK_STOP,       FK_LT_BRACKET,
  FK_YEN,       FK_RT_BRACKET, FK_CARET,      FK_CODA,
  FK_AT,        FK_A,          FK_B,          FK_C,
  FK_D,         FK_E,          FK_F,          FK_G,
  FK_H,         FK_I,          FK_J,          FK_K,
  FK_L,         FK_M,          FK_N,          FK_O,
  FK_P,         FK_Q,          FK_R,          FK_S,
  FK_T,         FK_U,          FK_V,          FK_W,
  FK_X,         FK_Y,          FK_Z,          -1,
  FK_SPACE,     -1,            FK_CARET,      -1
};

void fk_write_coded(unsigned char * s) {
  boolean p = true;
  boolean r = true;
  unsigned char ch;
  while ((ch = *s)) {
    if (ch >= 0x20 && ch < 0x7F) {
      if (ch == '}' || ch == ')') {
        p = true;
        r = true;
      } else if (ch == '{') {
        p = true;
        r = false;
      } else if (ch == '(') {
        p = false;
        r = true;
      } else {
        ch = pgm_read_byte(&codedValues[ch - 0x20]);
        if ((ch &= 0x7F) < 0x48) {
          if (p) {
            fk_write_packet(ch | FK_PRESSED);
            fk_delay(FK_WRITE_DELAY);
          }
          if (r) {
            fk_write_packet(ch | FK_RELEASED);
            fk_delay(FK_WRITE_DELAY);
          }
        }
      }
    }
    s++;
  }
}
